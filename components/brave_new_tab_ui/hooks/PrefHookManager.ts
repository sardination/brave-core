import { useCallback, useEffect, useState } from "react";

export type PrefListener<OnType, KeyType extends keyof OnType> = (name: KeyType, newValue: OnType[KeyType], oldValue?: OnType[KeyType]) => void;

export class PrefHookManager<T> {
    #listeners: Map<keyof T, PrefListener<T, keyof T>[]> = new Map();
    #lastValue?: T;
    #savePref: (key: keyof T, value: T[keyof T]) => void

    constructor(getInitialValue: () => Promise<T>, savePref: (key: keyof T, value: T[keyof T]) => void, addListener: (listener: (prefs: T) => void) => void) {
        this.#savePref = savePref;

        addListener((prefs: T) => {
            const oldPrefs = this.#lastValue ?? {} as Partial<T>;
            this.#lastValue = prefs;

            for (const [pref, listeners] of this.#listeners.entries()) {
                const newValue = prefs[pref];
                const oldValue = oldPrefs[pref];
                if (newValue === oldValue) continue;

                for (const listener of listeners)
                    listener(pref, newValue, oldValue);
            }
        });
        getInitialValue().then(v => this.#lastValue = v);
    }

    savePref<KeyType extends keyof T>(prefName: KeyType, value: T[KeyType]) {
        this.#savePref(prefName, value);
    }

    getPref<KeyType extends keyof T>(prefName: KeyType) {
        return this.#lastValue?.[prefName]
    }

    addPrefListener<KeyType extends keyof T>(prefName: KeyType, listener: PrefListener<T, KeyType>) {
        if (!this.#listeners.has(prefName))
            this.#listeners.set(prefName, []);
        this.#listeners.get(prefName)!.push(listener);
    }

    removePrefListener<KeyType extends keyof T>(prefName: KeyType, listener: PrefListener<T, KeyType>) {
        const prefListeners = this.#listeners.get(prefName);
        if (!prefListeners) return false;

        const index = prefListeners.indexOf(listener);
        prefListeners.splice(index, 1);
        return index !== -1;
    }
}

export const usePref = <OnType, PrefType extends keyof OnType>(prefManager: PrefHookManager<OnType>, pref: PrefType) => {
    const [value, setValue] = useState(prefManager.getPref(pref));
    const setPref = useCallback((value: OnType[PrefType]) => prefManager.savePref(pref, value), [prefManager, pref]);
    useEffect(() => {
        const handler: PrefListener<OnType, PrefType> = (_, newValue) => setValue(newValue as typeof value);
        prefManager.addPrefListener(pref, handler);
        return () => {
            prefManager.removePrefListener(pref, handler);
        }
    }, [prefManager, pref]);

    return [
        value,
        setPref
    ] as const;
}
