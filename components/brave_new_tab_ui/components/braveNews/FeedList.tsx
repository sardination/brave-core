
import * as React from "react";
import styled from "styled-components";
import { usePublishers } from "../../api/brave_news/news";
import Flex from "../Flex";

import FeedListEntry from "./FeedListEntry";

const Title = styled.span`
  font-size: 18px;
  font-weight: 800;
  line-height: 36px;
`;

const Subtitle = styled.span`
  font-weight: 500;
  font-size: 12px;
  color: #868e96;
`;

export default function FeedList() {
    const publishers = usePublishers({ enabled: true });

    return <div>
        <Flex direction="row" justify="space-between" align="center">
            <Title>Following</Title>
            <Subtitle>{publishers.length} sources</Subtitle>
        </Flex>
        <Flex direction="column">
            {publishers.map((p) => (
                <FeedListEntry key={p.publisherId} publisherId={p.publisherId} />
            ))}
        </Flex>
    </div>
}
