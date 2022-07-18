import { useCallback, useEffect, useMemo, useState } from "react";
import getBraveNewsController, { Publishers } from ".";
import { BraveNewsControllerRemote, FeedSearchResultItem, Publisher, PublisherType, UserEnabled } from "../../../../../out/Component/gen/brave/components/brave_today/common/brave_news.mojom.m";

type PublisherListener = (newValue: Publisher, oldValue: Publisher) => void;
type UpdatedListener = (publishers: Publishers, oldValue: Publishers) => void;

class BraveNewsApi {
    controller: BraveNewsControllerRemote;

    listeners: UpdatedListener[] = [];
    publisherListeners: Map<string, PublisherListener[]> = new Map();
    lastValue: Publishers = {};

    constructor() {
        this.controller = getBraveNewsController();
        this.updatePublishers();
    }

    getPublisher(publisherId: string) {
        return this.lastValue[publisherId];
    }

    getPublishers(enabled?: boolean, categoryId?: string) {
        let publishers = Object.values(this.lastValue);
        if (categoryId)
            publishers = publishers.filter(p => p.categoryName === categoryId);
        if (enabled !== undefined)
            publishers = publishers.filter(p => this.isPublisherEnabled(p.publisherId) === enabled);
        return publishers;
    }

    getCategories() {
        return Array.from(Object.values(this.lastValue).reduce((prev, next) => {
            if (next.categoryName)
                prev.add(next.categoryName);
            return prev;
        }, new Set<string>()));
    }

    async setPublisherPref(publisherId: string, status: UserEnabled) {
        const newValue = {
            ...this.lastValue,
            [publisherId]: {
                ...this.lastValue[publisherId],
                userEnabledStatus: status
            }
        }

        // We completely remove direct feeds when setting the UserEnabled status
        // to DISABLED.
        if (this.isDirectFeed(publisherId) && status == UserEnabled.DISABLED)
            delete newValue[publisherId];
        
        this.controller.setPublisherPref(publisherId, status);
        this.updatePublishers(newValue);
    }

    async subscribeToDirectFeed(feedUrl: string) {
        const { publishers } = await this.controller.subscribeToNewDirectFeed({ url: feedUrl });
        this.updatePublishers(publishers);
    }

    setPublisherEnabled(publisherId: string, enabled: boolean) {
        this.setPublisherPref(publisherId, enabled ? UserEnabled.ENABLED : UserEnabled.DISABLED);
    }

    isPublisherEnabled(publisherId: string) {
        const publisher = this.lastValue[publisherId];
        if (!publisher) return false;
        return publisher.isEnabled
            && publisher.userEnabledStatus == UserEnabled.NOT_MODIFIED
            || publisher.userEnabledStatus == UserEnabled.ENABLED;
    }

    isDirectFeed(publisherId: string) {
        const publisher = this.lastValue[publisherId];
        if (!publisher) return false;
        return publisher.type == PublisherType.DIRECT_SOURCE;
    }

    async updatePublishers(newPublishers?: Publishers) {
        if (!newPublishers)
            ({ publishers: newPublishers } = await this.controller.getPublishers());

        const oldValue = this.lastValue;
        this.lastValue = newPublishers!;

        this.notifyListeners(newPublishers!, oldValue);
    }

    addListener(listener: UpdatedListener) {
        this.listeners.push(listener);
    }

    removeListener(listener: UpdatedListener) {
        const index = this.listeners.indexOf(listener);
        this.listeners.splice(index, 1);
    }

    addPublisherListener(publisherId: string, listener: PublisherListener) {
        if (!this.publisherListeners.has(publisherId))
            this.publisherListeners.set(publisherId, []);

        this.publisherListeners.get(publisherId)!.push(listener);
    }

    removePublisherListener(publisherId: string, listener: PublisherListener) {
        const listeners = this.publisherListeners.get(publisherId);
        if (!listeners) return;

        const index = listeners.indexOf(listener);
        listeners.splice(index, 1);

        if (listeners.length === 0)
            this.publisherListeners.delete(publisherId);
    }

    notifyListeners(newValue: Publishers, oldValue: Publishers) {
        for (const listener of this.listeners)
            listener(newValue, oldValue);

        this.notifyPublisherListeners(newValue, oldValue);
    }

    notifyPublisherListeners(newValue: Publishers, oldValue: Publishers) {
        for (const publisher of Object.values(newValue)) {
            const oldPublisher = oldValue[publisher.publisherId];
            if (publisher === oldPublisher) continue;

            for (const listener of this.publisherListeners.get(publisher.publisherId) ?? [])
                listener(publisher, oldPublisher);
        }
    }
}

export const api = new BraveNewsApi();
window['api'] = api;

export const useCategories = () => {
    const [categories, setCategories] = useState<string[]>([]);
    useEffect(() => {
        const handler = () => setCategories(api.getCategories());
        handler();
        api.addListener(handler);
        return () => {
            api.removeListener(handler);
        }
    }, []);

    return categories;
}

export const usePublishers = (options?: { enabled?: boolean, categoryId?: string }) => {
    const [publishers, setPublishers] = useState<Publisher[]>([]);
    useEffect(() => {
        const handler = () => setPublishers(api.getPublishers(options?.enabled, options?.categoryId));
        handler();

        api.addListener(handler);
        return () => {
            api.removeListener(handler);
        }
    }, [options?.enabled, options?.categoryId]);

    const sorted = useMemo(() => publishers.sort((a, b) => a.publisherName.localeCompare(b.publisherName)), [publishers]);
    return sorted;
}

export const usePublisher = (publisherId: string) => {
    const [publisher, setPublisher] = useState(api.getPublisher(publisherId));

    useEffect(() => {
        const handler: PublisherListener = newValue => {
            setPublisher(newValue);
        };
        api.addPublisherListener(publisherId, handler);

        return () => {
            api.removePublisherListener(publisherId, handler);
        }
    }, [publisherId]);

    const setEnabled = useCallback((enabled: boolean) => {
        api.setPublisherEnabled(publisherId, enabled);
    }, [publisherId])

    const enabled = api.isPublisherEnabled(publisherId);

    return {
        publisher,
        enabled,
        setEnabled
    }
}

export const useSearchResults = (query: string) => {
    // We're not interested in casing.
    query = query.toLocaleLowerCase();

    const publishers = usePublishers();
    const feedResults = useMemo(() => query
        ? publishers.filter(p => {
            if (p.publisherName.toLocaleLowerCase().includes(query))
                return true;
            if (p.categoryName.toLocaleLowerCase().includes(query))
                return true;
            if (p.feedSource?.url?.toLocaleLowerCase().includes(query))
                return true;
            return false;
        }).sort((a, b) => a.publisherName.localeCompare(b.publisherName))
        : [], [query, publishers]);

    const [directResults, setDirectResults] = useState<FeedSearchResultItem[]>([]);
    const [loading, setLoading] = useState(false);

    useEffect(() => {
        setDirectResults([]);

        let cancelled = false;
        let url: URL | undefined;
        try { url = new URL(query) } catch { }
        if (!url) return;

        setLoading(true);

        api.controller.findFeeds({ url: url.toString() }).then(({ results }) => {
            if (cancelled) return;
            setLoading(false);
            setDirectResults(results);
        });
        return () => {
            cancelled = true;
        }
    }, [query]);

    const dedupedDirectResults = useMemo(() => {
        const allFeeds = new Set(publishers.map(p => p.feedSource.url));
        return directResults.filter(d => !allFeeds.has(d.feedUrl.url));
    }, [directResults, publishers]);

    return {
        loading,
        feedResults,
        directResults: dedupedDirectResults
    }
}
