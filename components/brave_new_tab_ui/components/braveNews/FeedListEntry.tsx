import * as React from 'react';
import styled from 'styled-components';
import Flex from "../Flex";
import { Heart, HeartOutline } from './Icons';
import { usePublisher } from '../../api/brave_news/news';

interface Props {
    publisherId: string;
}

const Container = styled(Flex)`
    padding: 10px 0;
    cursor: pointer;
    
    :hover {
        opacity: 0.5;
    }
`

const FavIcon = styled.img`
    width: 24px;
    height: 24px;
    border-radius: 100px;
`
const ToggleButton = styled.button`
    all: unset;
    cursor: pointer;
    color: var(--interactive5);
`

const Text = styled.span`
    font-size: 14px;
    font-weight: 500;
`

export default function FeedListEntry(props: Props) {
    const { publisher, enabled, setEnabled } = usePublisher(props.publisherId);

    return <Container direction="row" justify="space-between" align='center' onClick={() => setEnabled(!enabled)}>
        <Flex align='center' gap={8}>
            <FavIcon src={''} />
            <Text>{publisher.publisherName}</Text>
        </Flex>
        <ToggleButton>
            {enabled ? Heart : HeartOutline}
        </ToggleButton>
    </Container>
}
