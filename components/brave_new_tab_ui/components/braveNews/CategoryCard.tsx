import styled, { css } from 'styled-components';
import * as React from 'react';
import Flex from "../Flex";
import { useBraveNews } from './Context';

const Container = styled(Flex) <{ height: number, backgroundColor: string, backgroundUrl?: string }>`
    height: ${p => `${p.height}px`};
    font-weight: 600;
    font-size: 14px;
    border-radius: 8px;
    background: ${p => p.backgroundColor};
    ${p => p.backgroundUrl && css`
        background-image: url("${p.backgroundUrl}");
    `}
    padding: 16px 20px;
    color: white;

    :hover {
        opacity: 0.8;
    }
`

interface Props {
    categoryId: string;
    text: string;
    backgroundColor: string;
    backgroundUrl?: string;
    icon?: React.ReactNode;
}

export default function SourceCard(props: Props) {
    const { setPage } = useBraveNews();

    return <Container
        onClick={() => setPage(`category/${props.categoryId}`)}
        direction='column'
        justify={props.icon ? 'end' : 'center'}
        align={props.icon ? 'start' : 'center'}
        backgroundColor={props.backgroundColor}
        backgroundUrl={props.backgroundUrl}
        height={props.icon ? 102 : 80}
    >
        {props.icon}
        {props.text}
    </Container>
}
