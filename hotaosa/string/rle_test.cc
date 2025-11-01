#include "hotaosa/string/rle.h"

#include <string>
#include <string_view>
#include <vector>

#include "gtest/gtest.h"

namespace hotaosa {
namespace {

TEST(RleTest, CompressesString) {
  const std::string input = "aaabbc";
  const auto encoded = Rle(input);
  const std::vector expected = {
      RunLengthBlock<char>{'a', 3},
      RunLengthBlock<char>{'b', 2},
      RunLengthBlock<char>{'c', 1},
  };
  EXPECT_EQ(encoded, expected);
}

TEST(RleTest, HandlesNumericSequence) {
  const std::vector<int> input = {1, 1, 2, 3, 3, 3, 3};
  const auto encoded = Rle(input);
  const std::vector expected = {
      RunLengthBlock<int>{1, 2},
      RunLengthBlock<int>{2, 1},
      RunLengthBlock<int>{3, 4},
  };
  EXPECT_EQ(encoded, expected);
}

TEST(RleTest, HandlesEmptyRange) {
  const std::string_view input;
  const auto encoded = Rle(input);
  EXPECT_TRUE(encoded.empty());
}

}  // namespace
}  // namespace hotaosa
