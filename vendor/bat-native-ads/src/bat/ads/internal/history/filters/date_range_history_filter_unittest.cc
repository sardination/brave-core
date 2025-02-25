/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ads/internal/history/filters/date_range_history_filter.h"

#include "base/containers/circular_deque.h"
#include "base/time/time.h"
#include "bat/ads/history_info.h"
#include "bat/ads/history_item_info.h"
#include "bat/ads/internal/base/containers/container_util.h"
#include "bat/ads/internal/base/unittest/unittest_time_util.h"
#include "testing/gtest/include/gtest/gtest.h"

// npm run test -- brave_unit_tests --filter=BatAds*

namespace ads {

namespace {

base::circular_deque<HistoryItemInfo> GetHistory() {
  base::circular_deque<HistoryItemInfo> history;

  HistoryItemInfo history_item;
  history_item.created_at = base::Time::FromDoubleT(333333333);
  history.push_back(history_item);
  history_item.created_at = base::Time::FromDoubleT(444444444);
  history.push_back(history_item);
  history_item.created_at = base::Time::FromDoubleT(222222222);
  history.push_back(history_item);
  history_item.created_at = base::Time::FromDoubleT(666666666);
  history.push_back(history_item);
  history_item.created_at = base::Time::FromDoubleT(555555555);
  history.push_back(history_item);

  return history;
}

}  // namespace

TEST(BatAdsDateRangeHistoryFilterTest,
     FilterHistoryFromTimestamp444444444ToDistantFuture) {
  // Arrange
  base::circular_deque<HistoryItemInfo> history = GetHistory();

  const base::Time from_time = base::Time::FromDoubleT(444444444);
  const base::Time to_time = DistantFuture();

  // Act
  DateRangeHistoryFilter filter(from_time, to_time);
  history = filter.Apply(history);

  // Assert
  base::circular_deque<HistoryItemInfo> expected_history;

  HistoryItemInfo history_item;
  history_item.created_at = base::Time::FromDoubleT(444444444);
  expected_history.push_back(history_item);
  history_item.created_at = base::Time::FromDoubleT(666666666);
  expected_history.push_back(history_item);
  history_item.created_at = base::Time::FromDoubleT(555555555);
  expected_history.push_back(history_item);

  EXPECT_TRUE(IsEqualContainers(expected_history, history));
}

TEST(BatAdsDateRangeHistoryFilterTest,
     FilterHistoryFromTimestamp777777777ToDistantFuture) {
  // Arrange
  base::circular_deque<HistoryItemInfo> history = GetHistory();

  const base::Time from_time = base::Time::FromDoubleT(777777777);
  const base::Time to_time = DistantFuture();

  // Act
  DateRangeHistoryFilter filter(from_time, to_time);
  history = filter.Apply(history);

  // Assert
  const base::circular_deque<HistoryItemInfo> expected_history = {};

  EXPECT_TRUE(IsEqualContainers(expected_history, history));
}

TEST(BatAdsDateRangeHistoryFilterTest,
     FilterHistoryFromDistantPastToTimestamp444444444) {
  // Arrange
  base::circular_deque<HistoryItemInfo> history = GetHistory();

  const base::Time from_time = DistantPast();
  const base::Time to_time = base::Time::FromDoubleT(444444444);

  // Act
  DateRangeHistoryFilter filter(from_time, to_time);
  history = filter.Apply(history);

  // Assert
  base::circular_deque<HistoryItemInfo> expected_history;
  HistoryItemInfo history_item;
  history_item.created_at = base::Time::FromDoubleT(333333333);
  expected_history.push_back(history_item);
  history_item.created_at = base::Time::FromDoubleT(444444444);
  expected_history.push_back(history_item);
  history_item.created_at = base::Time::FromDoubleT(222222222);
  expected_history.push_back(history_item);

  EXPECT_TRUE(IsEqualContainers(expected_history, history));
}

TEST(BatAdsDateRangeHistoryFilterTest,
     FilterHistoryFromDistancePastToTimestamp111111111) {
  // Arrange
  base::circular_deque<HistoryItemInfo> history = GetHistory();

  const base::Time from_time = DistantPast();
  const base::Time to_time = base::Time::FromDoubleT(111111111);

  // Act
  DateRangeHistoryFilter filter(from_time, to_time);
  history = filter.Apply(history);

  // Assert
  const base::circular_deque<HistoryItemInfo> expected_history = {};

  EXPECT_TRUE(IsEqualContainers(expected_history, history));
}

TEST(BatAdsDateRangeHistoryFilterTest,
     FilterHistoryFromDistantPastToDistantFuture) {
  // Arrange
  base::circular_deque<HistoryItemInfo> history = GetHistory();

  const base::Time from_time = DistantPast();
  const base::Time to_time = DistantFuture();

  // Act
  DateRangeHistoryFilter filter(from_time, to_time);
  history = filter.Apply(history);

  // Assert
  base::circular_deque<HistoryItemInfo> expected_history;
  HistoryItemInfo history_item;
  history_item.created_at = base::Time::FromDoubleT(333333333);
  expected_history.push_back(history_item);
  history_item.created_at = base::Time::FromDoubleT(444444444);
  expected_history.push_back(history_item);
  history_item.created_at = base::Time::FromDoubleT(222222222);
  expected_history.push_back(history_item);
  history_item.created_at = base::Time::FromDoubleT(666666666);
  expected_history.push_back(history_item);
  history_item.created_at = base::Time::FromDoubleT(555555555);
  expected_history.push_back(history_item);

  EXPECT_TRUE(IsEqualContainers(expected_history, history));
}

TEST(BatAdsDateRangeHistoryFilterTest,
     FilterHistoryFromDistantFutureToDistantPast) {
  // Arrange
  base::circular_deque<HistoryItemInfo> history = GetHistory();

  const base::Time from_time = DistantFuture();
  const base::Time to_time = DistantPast();

  // Act
  DateRangeHistoryFilter filter(from_time, to_time);
  history = filter.Apply(history);

  // Assert
  const base::circular_deque<HistoryItemInfo> expected_history = {};

  EXPECT_TRUE(IsEqualContainers(expected_history, history));
}

TEST(BatAdsDateRangeHistoryFilterTest, FilterEmptyHistory) {
  // Arrange
  base::circular_deque<HistoryItemInfo> history;

  const base::Time from_time = base::Time::FromDoubleT(444444444);
  const base::Time to_time = DistantFuture();

  // Act
  DateRangeHistoryFilter filter(from_time, to_time);
  history = filter.Apply(history);

  // Assert
  const base::circular_deque<HistoryItemInfo> expected_history = {};

  EXPECT_TRUE(IsEqualContainers(expected_history, history));
}

}  // namespace ads
