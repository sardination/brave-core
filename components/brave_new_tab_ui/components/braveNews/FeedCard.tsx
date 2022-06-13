import { useState } from "react";
import * as React from "react";
import styled, { keyframes } from "styled-components";
import Flex from "../Flex";
import FollowButton from "./FollowButton";
import { Heart, HeartOutline } from "./Icons";
import { usePublisher } from "../../api/brave_news/news";

const Container = styled(Flex)`
`;

const Card = styled('div') <{ backgroundColor?: string, backgroundImage?: string }>`
    position: relative;
    height: 80px;
    background-color: ${p => p.backgroundColor};
    border-radius: 16px;
    overflow: hidden;
    box-shadow: 0px 0px 16px 0px #63696E2E;
    ${p => p.backgroundImage && `background-image: url('${p.backgroundImage}');`}
`

const StyledFollowButton = styled(FollowButton)`
    position: absolute;
    right: 8px;
    top: 8px;
`

const Name = styled.span`
    font-size: 14px;
    font-weight: 600;
`

const Pulse = keyframes`
    0% {
        pointer-events: auto;
        opacity: 0;
    }
    5% { opacity: 1; }
    80% { opacity: 1; }
    99% {
        pointer-events: auto;
    }
    100% { 
        pointer-events: none;
        opacity: 0;
    }
`

const HeartOverlay = styled(Flex)`
    pointer-events: none;
    background: white;
    color: #aeb1c2;
    position: absolute;
    top: 0;
    bottom: 0;
    left: 0;
    right: 0;
    opacity: 0;
    animation: ${Pulse} 2s ease-in-out;
`

const HeartContainer = styled.div`
    width: 32px;
    height: 32px;

    > svg {
        width: 100%;
        height: 100%;
    }
`

export default function FeedCard(props: {
    publisherId: string;
    backgroundColor?: string;
    backgroundImage?: string;
}) {
    const { publisher, enabled, setEnabled } = usePublisher(props.publisherId);
    const [toggled, setToggled] = useState(false);
    const toggle = () => {
        setToggled(true);
        setEnabled(!enabled);
    }
    return <Container direction="column" gap={8}>
        <Card backgroundColor={props.backgroundColor} backgroundImage={props.backgroundImage}>
            <StyledFollowButton following={enabled} onClick={toggle} />

            {/*
                Use whether or not we're following this element as the key, so
                React remounts the component when we toggle following and plays
                the animation.
                
                We don't display the overlay unless we've toggled this publisher
                so we don't play the pulse animation on first load.
            */}
            {toggled && <HeartOverlay key={enabled + ''} align="center" justify="center">
                <HeartContainer>
                    {enabled ? Heart : HeartOutline}
                </HeartContainer>
            </HeartOverlay>}
        </Card>
        <Name>
            {publisher.publisherName}
        </Name>
    </Container>
}

export function DirectFeedCard(props: {
    feedUrl: string;
    title: string;
}) {
    return <Container direction="column" gap={8}>
        <Card>
            <StyledFollowButton following={false} onClick={console.log}/>
        </Card>
        <Name>
            {props.title}
        </Name>
    </Container>
}
