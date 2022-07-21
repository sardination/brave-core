import * as React from 'react';
import { getLocale } from '../../../common/locale';
import { useSearchResults } from '../../api/brave_news/news';
import DiscoverSection from './DiscoverSection';
import FeedCard, { DirectFeedCard } from './FeedCard';

interface Props {
    query: string;
}

export default function SearchResults(props: Props) {
    const { loading, directResults, feedResults } = useSearchResults(props.query);
    const noResults = props.query
        && !loading
        && !directResults.length
        && !feedResults.length;

    return <div>
        {loading
            ? <span>{getLocale('braveNewsSearchResultsLoading')}</span>
            : <>
                {!!directResults.length
                    && <DiscoverSection name={getLocale('braveNewsSearchResultsDirectResults')}>
                        {directResults.map(r => <DirectFeedCard key={r.feedUrl.url} feedUrl={r.feedUrl.url} title={r.feedTitle} />)}
                    </DiscoverSection>}
            </>}
        {!!feedResults.length && <DiscoverSection name={getLocale('braveNewsSearchResultsLocalResults')}>
            {feedResults.map(r => <FeedCard key={r.publisherId} publisherId={r.publisherId} />)}
        </DiscoverSection>}
        {noResults && <div>
            {getLocale('braveNewsSearchResultsNoResults')}
        </div>}
    </div>

}
