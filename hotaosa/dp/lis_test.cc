#include "hotaosa/dp/lis.h"

#include <ranges>
#include <string>
#include <vector>

#include "gtest/gtest.h"

namespace hotaosa {
namespace {

TEST(LISTest, LengthStrict) {
  const std::vector<int> data = {3, 1, 2, 1, 8, 5, 6};
  EXPECT_EQ(LisLength(data), 4);
}

TEST(LISTest, EndingLengthsStrict) {
  const std::vector<int> data = {3, 1, 2, 1, 8, 5, 6};
  const std::vector<int> expected = {1, 1, 2, 1, 3, 3, 4};
  EXPECT_EQ(LisEndingLengths(data), expected);
}

TEST(LISTest, LengthNonStrict) {
  const std::vector<int> data = {1, 1, 1};
  EXPECT_EQ(LisLength(data, std::ranges::less{}, LisMode::kNonStrict),
            3);
}

TEST(LISTest, EndingLengthsNonStrict) {
  const std::vector<int> data = {1, 1, 1};
  const std::vector<int> expected = {1, 2, 3};
  EXPECT_EQ(LisEndingLengths(data, std::ranges::less{}, LisMode::kNonStrict),
            expected);
}

TEST(LISTest, SequenceReconstruction) {
  const std::vector<int> data = {3, 1, 2, 1, 8, 5, 6};
  const auto subseq = Lis(data);
  const std::vector<int> expected = {1, 2, 5, 6};
  EXPECT_EQ(subseq, expected);
}

TEST(LISTest, IndicesStrict) {
  const std::vector<int> data = {3, 1, 2, 1, 8, 5, 6};
  const std::vector<int> expected = {1, 2, 5, 6};
  EXPECT_EQ(LisIndices(data), expected);
}

TEST(LISTest, StartingLengthsStrict) {
  const std::vector<int> data = {3, 1, 2, 1, 8, 5, 6};
  const std::vector<int> expected = {3, 4, 3, 3, 1, 2, 1};
  EXPECT_EQ(LisStartingLengths(data), expected);
}

TEST(LISTest, WorksOnString) {
  const std::string data = "atcoder";
  EXPECT_EQ(LisLength(data), 5);
}

TEST(LISTest, NonStrictSequence) {
  const std::vector<int> data = {1, 1, 1};
  const auto subseq = Lis(data, std::ranges::less{}, LisMode::kNonStrict);
  const std::vector<int> expected = {1, 1, 1};
  EXPECT_EQ(subseq, expected);
}

TEST(LISTest, IndicesNonStrict) {
  const std::vector<int> data = {1, 1, 1};
  const std::vector<int> expected = {0, 1, 2};
  EXPECT_EQ(LisIndices(data, std::ranges::less{}, LisMode::kNonStrict),
            expected);
}

TEST(LISTest, StartingLengthsNonStrict) {
  const std::vector<int> data = {1, 1, 1};
  const std::vector<int> expected = {3, 2, 1};
  EXPECT_EQ(LisStartingLengths(data, std::ranges::less{}, LisMode::kNonStrict),
            expected);
}

}  // namespace
}  // namespace hotaosa
