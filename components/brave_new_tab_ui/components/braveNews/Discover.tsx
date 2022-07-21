import Button from '$web-components/button'
import TextInput from '$web-components/input'
import * as React from 'react'
import { useState } from 'react'
import styled from 'styled-components'
import { getLocale } from '../../../common/locale'
import { useCategories, usePublishers } from '../../api/brave_news/news'
import Flex from '../Flex'
import CategoryCard from './CategoryCard'
import { getCardColor } from './colors'
import DiscoverSection from './DiscoverSection'
import FeedCard from './FeedCard'
import SearchResults from './SearchResults'

const Header = styled.span`
    font-size: 24px;
    font-weight: 600;
    padding: 12px 0;
`

const SearchInput = styled(TextInput)`
    margin: 16px 0;
    border-radius: 4px;
    --interactive8: #AEB1C2;
    --focus-border: #737ADE;
`

const LoadMoreButton = styled(Button)`
    grid-column: 2;
`

// The default number of category cards to show.
const DEFAULT_NUM_CATEGORIES = 3;

// Some hard coded categories, which are shown in the UI.

export default function Discover(props: {}) {
    const categories = useCategories();
    const [showingAllCategories, setShowingAllCategories] = React.useState(false);
    const [query, setQuery] = useState('');
    const publishers = usePublishers();

    return <Flex direction='column'>
        <Header>Discover</Header>
        <SearchInput type="search" placeholder={getLocale('braveNewsSearchPlaceholderLabel')} value={query} onInput={e => setQuery(e.currentTarget.value)} />
        <SearchResults query={query} />
        {!query && <>
            <DiscoverSection name={getLocale('braveNewsBrowseByCategoryHeader')}>
                {categories
                    // If we're showing all categories, there's no end to the slice.
                    // Otherwise, just show the default number.
                    .slice(0, showingAllCategories
                        ? undefined
                        : DEFAULT_NUM_CATEGORIES)
                    .map(c => <CategoryCard key={c} categoryId={c} text={c} backgroundColor={getCardColor(c)} />)}
                {!showingAllCategories
                    && <LoadMoreButton onClick={() => setShowingAllCategories(true)}>
                        {getLocale('braveNewsLoadMoreCategoriesButton')}
                    </LoadMoreButton>}
            </DiscoverSection>
            <DiscoverSection name={getLocale('braveNewsAllSourcesHeader')}>
                {publishers.map(p => <FeedCard key={p.publisherId} publisherId={p.publisherId} />)}
            </DiscoverSection>
        </>}
    </Flex>
}
