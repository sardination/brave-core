import * as React from 'react';
import styled from 'styled-components';
import Flex from '../Flex';

interface Props {
    name: string;
    subtitle?: React.ReactNode;
    sectionId: string;
    children?: React.ReactNode;
}

const Container = styled(Flex)`
    padding: 16px 0;
    cursor: pointer;
`

const Header = styled.span`
    font-weight: 600;
    font-size: 16px;
    margin: 8px 0;
`

const Link = styled.a`
    color: #4C54D2;
    font-size: 12px;
    font-weight: 600;
`

const Subtitle = styled.span`
    font-size: 12px;
`

const ItemsContainer = styled.div`
    margin: 8px 0;
    display: grid;
    grid-template-columns: repeat(3, minmax(0, 208px));
    gap: 16px;
`

export default function DiscoverSection(props: Props) {
    return <Container direction='column'>
        <Flex direction='row' gap={8} align='center'>
            <Header>{props.name}</Header>
            <Link>View All</Link>
        </Flex>
        {props.subtitle && <Subtitle>
            {props.subtitle}
        </Subtitle>}
        <ItemsContainer>
            {props.children}
        </ItemsContainer>
    </Container>
}
