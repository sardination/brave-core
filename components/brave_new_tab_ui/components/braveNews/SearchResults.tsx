import * as React from 'react';
import styled from 'styled-components';
import { useSearchResults } from '../../api/brave_news/news';
import DiscoverSection from './DiscoverSection';
import FeedCard, { DirectFeedCard } from './FeedCard';

interface Props {
    query: string;
}

const Grid = styled.div`
    display: grid;
    grid-template-columns: repeat(3, 1fr);
`

export default function SearchResults(props: Props) {
    const { loading, directResults, feedResults } = useSearchResults(props.query);
    const noResults = props.query
        && !loading
        && !directResults.length
        && !feedResults.length;
    return <div>
        {loading
            ? <span>Loading...</span>
            : <>
                {!directResults.length
                    && <DiscoverSection name='Direct Feeds'>
                        {directResults.map(r => <DirectFeedCard key={r.feedUrl.url} feedUrl={r.feedUrl.url} title={r.feedTitle} />)}
                    </DiscoverSection>}

                {!!feedResults.length && <DiscoverSection name="">
                    {feedResults.map(r => <FeedCard key={r.publisherId} publisherId={r.publisherId} />)}
                </DiscoverSection>}
            </>}
        {noResults && <div>
            There's nothing here :'(
        </div>}
    </div>

}
