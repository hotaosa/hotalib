#include "hotaosa/interval/interval_set.h"

#include <gtest/gtest.h>

#include <utility>
#include <vector>

namespace {

TEST(IntervalSetTest, AddAndQueryRightOpen) {
  hotaosa::IntervalSet<int> intervals;
  EXPECT_TRUE(intervals.Empty());

  intervals.Add(2, 5);
  EXPECT_TRUE(intervals.Contains(2));
  EXPECT_TRUE(intervals.Contains(4));
  EXPECT_FALSE(intervals.Contains(5));
  EXPECT_EQ(intervals.Size(), 3);

  intervals.Add(5, 8);
  // [2,5) U [5,8) merges into [2,8) thanks to adjacency.
  EXPECT_TRUE(intervals.Contains(7));
  EXPECT_FALSE(intervals.Contains(8));
  EXPECT_EQ(intervals.Size(), 6);
}

TEST(IntervalSetTest, AssignReplacesExistingRange) {
  hotaosa::IntervalSet<int> intervals;
  intervals.Add(0, 10);
  intervals.Assign(3, 6);
  EXPECT_FALSE(intervals.Contains(2));
  EXPECT_TRUE(intervals.Contains(4));
  EXPECT_FALSE(intervals.Contains(9));
  EXPECT_EQ(intervals.Size(), 3);
}

TEST(IntervalSetTest, EraseBreaksIntervals) {
  hotaosa::IntervalSet<int> intervals;
  intervals.Add(0, 10);
  intervals.Erase(3, 7);

  EXPECT_TRUE(intervals.Contains(2));
  EXPECT_FALSE(intervals.Contains(3));
  EXPECT_FALSE(intervals.Contains(6));
  EXPECT_TRUE(intervals.Contains(8));

  // We expect two disjoint intervals: [0,3) and [7,10).
  std::vector<std::pair<int, int>> segments;
  for (const auto& interval : intervals) {
    segments.emplace_back(interval.lower(), interval.upper());
  }
  ASSERT_EQ(segments.size(), 2);
  EXPECT_EQ(segments[0], std::make_pair(0, 3));
  EXPECT_EQ(segments[1], std::make_pair(7, 10));
}

TEST(IntervalSetTest, CoversRightOpen) {
  hotaosa::IntervalSet<int> intervals;
  intervals.Add(10, 20);
  EXPECT_TRUE(intervals.Covers(12, 15));
  EXPECT_FALSE(intervals.Covers(5, 12));
  EXPECT_FALSE(intervals.Covers(15, 22));
}

TEST(IntervalSetTest, PointOperations) {
  hotaosa::IntervalSet<int> intervals;
  intervals.Add(3);
  EXPECT_TRUE(intervals.Contains(3));
  EXPECT_EQ(intervals.Size(), 1);

  intervals.Add(4);
  EXPECT_TRUE(intervals.Contains(4));
  EXPECT_EQ(intervals.Size(), 2);

  intervals.Add(3);  // duplicate add is idempotent
  EXPECT_EQ(intervals.Size(), 2);

  intervals.Erase(3);
  EXPECT_FALSE(intervals.Contains(3));
  EXPECT_TRUE(intervals.Contains(4));
  EXPECT_EQ(intervals.Size(), 1);

  intervals.Assign(10);
  EXPECT_TRUE(intervals.Contains(10));
  EXPECT_FALSE(intervals.Contains(4));
  EXPECT_EQ(intervals.Size(), 1);
}

TEST(IntervalSetTest, MexQueries) {
  hotaosa::IntervalSet<int> intervals;
  EXPECT_EQ(intervals.Mex(), 0);
  EXPECT_EQ(intervals.Mex(-10), 0);

  intervals.Add(0, 3);
  EXPECT_EQ(intervals.Mex(), 3);

  intervals.Add(3);
  EXPECT_EQ(intervals.Mex(), 4);

  EXPECT_EQ(intervals.Mex(5), 5);
  intervals.Add(5, 12);
  EXPECT_EQ(intervals.Mex(5), 12);

  intervals.Add(12, 15);
  EXPECT_EQ(intervals.Mex(5), 15);
  EXPECT_EQ(intervals.Mex(12), 15);
}

}  // namespace
