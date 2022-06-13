import Button from "$web-components/button";
import Toggle from "$web-components/toggle";
import * as React from "react";
import styled from "styled-components";
import { api, usePublishers } from "../../api/brave_news/news";
import Flex from "../Flex";
import { useBraveNews } from "./Context";
import FeedCard from "./FeedCard";
import FollowButton from "./FollowButton";
import { BackArrow } from "./Icons";

const Container = styled.div`
    height: 100%;
    overflow: auto;
`

const BackButton = styled(Button)`
    justify-self: start;
`

const Header = styled(Flex)`
    display: grid;
    grid-template-columns: repeat(3, 1fr);
    grid-template-rows: auto;
    justify-self: center;
    margin-bottom: 36px;
`

const HeaderText = styled(Flex)`
    font-weight: 500;
    font-size: 16px;
`

const FeedCardsContainer = styled('div')`
    display: grid;
    grid-template-columns: repeat(3, auto);
    grid-template-columns: repeat(auto-fill, auto);
    gap: 40px 16px;
    margin-top: 12px;
`

const colors = [
    'gray',
    'green',
    'salmon'
]

export default function BrowseCategory(props: { categoryId: string }) {
    const { setPage } = useBraveNews()
    const publishers = usePublishers({ categoryId: props.categoryId })
    const allSelected = !publishers.some(p => !api.isPublisherEnabled(p.publisherId));

    const toggleAllSelected = () => {
        for (const publisher of publishers)
            api.setPublisherEnabled(publisher.publisherId, !allSelected);
    }

    return <Container>
        <Header direction="row" align="center">
            <BackButton onClick={() => setPage('news')}>
                {BackArrow} Back
            </BackButton>
            <HeaderText>
                {props.categoryId}
            </HeaderText>
        </Header>
        <Flex direction="row" justify="space-between" align="center">
            <Flex direction="row" align="center" gap={8}>
                <Toggle isOn={allSelected} onChange={toggleAllSelected} />
                <span>Select {allSelected ? 'None' : 'All'}</span>
            </Flex>
            <FollowButton following={false} onClick={console.log} />
        </Flex>
        <FeedCardsContainer>
            {publishers.map((p) => <FeedCard
                key={p.publisherId}
                publisherId={p.publisherId}
                backgroundColor={colors[p.publisherName.length % colors.length]} />)}
        </FeedCardsContainer>
    </Container>
}
