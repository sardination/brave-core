import { useMemo, useState } from "react";
import * as React from "react";

type NewsPage = null
    | 'news'
    | `${typeof categoryPrefix}${string}`;

interface BraveNewsContext {
    page: NewsPage;
    setPage: (page: NewsPage) => void;
}

export const BraveNewsContext = React.createContext<BraveNewsContext>({
    page: 'news',
    setPage: () => {}
});

export function BraveNewsContextProvider(props: { children: React.ReactNode }) {
    const [page, setPage] = useState<NewsPage>(null);
    const context = useMemo(() => ({
        page,
        setPage
    }), [page]);
    return <BraveNewsContext.Provider value={context}>
        {props.children}
    </BraveNewsContext.Provider>
}

export const useBraveNews = () => {
    return React.useContext(BraveNewsContext);
}

const categoryPrefix = 'category/';
export const useCurrentCategory = () => {
    const { page } = useBraveNews();
    if (!page?.startsWith(categoryPrefix))
        return null;

    return page.substring(categoryPrefix.length);
}
