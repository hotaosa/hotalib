#include "hotaosa/string/run_length_encoding.h"

#include <string>
#include <string_view>
#include <vector>

#include "gtest/gtest.h"

namespace hotaosa {
namespace {

TEST(RunLengthEncodeTest, CompressesString) {
  const std::string input = "aaabbc";
  const auto encoded = RunLengthEncode(input);
  const std::vector expected = {
      RunLengthBlock<char>{'a', 3},
      RunLengthBlock<char>{'b', 2},
      RunLengthBlock<char>{'c', 1},
  };
  EXPECT_EQ(encoded, expected);
}

TEST(RunLengthEncodeTest, HandlesNumericSequence) {
  const std::vector<int> input = {1, 1, 2, 3, 3, 3, 3};
  const auto encoded = RunLengthEncode(input);
  const std::vector expected = {
      RunLengthBlock<int>{1, 2},
      RunLengthBlock<int>{2, 1},
      RunLengthBlock<int>{3, 4},
  };
  EXPECT_EQ(encoded, expected);
}

TEST(RunLengthEncodeTest, HandlesEmptyRange) {
  const std::string_view input;
  const auto encoded = RunLengthEncode(input);
  EXPECT_TRUE(encoded.empty());
}

}  // namespace
}  // namespace hotaosa
