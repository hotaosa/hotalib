#ifndef HOTAOSA_STRING_RLE_H_
#define HOTAOSA_STRING_RLE_H_

#include <concepts>
#include <ranges>
#include <vector>

namespace hotaosa {

// Represents one block in a run-length encoded sequence.
template <typename T>
struct RunLengthBlock {
  T value;
  int count;

  friend constexpr bool operator==(const RunLengthBlock&,
                                   const RunLengthBlock&) = default;
};

// Compresses consecutive equal elements of `range` into run-length blocks.
// Complexity: O(N) comparisons and copies, where N is the range length.
template <std::ranges::forward_range Range>
auto Rle(const Range& range)
    -> std::vector<RunLengthBlock<std::ranges::range_value_t<Range>>>
  requires std::copy_constructible<std::ranges::range_value_t<Range>> &&
           std::equality_comparable<std::ranges::range_value_t<Range>>
{
  using ValueType = std::ranges::range_value_t<Range>;
  std::vector<RunLengthBlock<ValueType>> result;
  if constexpr (std::ranges::sized_range<Range>) {
    result.reserve(std::ranges::size(range));
  }
  auto it = std::ranges::begin(range);
  const auto end = std::ranges::end(range);
  if (it == end) {
    return result;
  }
  ValueType current = *it;
  int count = 1;
  for (++it; it != end; ++it) {
    if (*it == current) {
      ++count;
      continue;
    }
    result.emplace_back(RunLengthBlock<ValueType>{current, count});
    current = *it;
    count = 1;
  }
  result.emplace_back(RunLengthBlock<ValueType>{current, count});
  return result;
}

}  // namespace hotaosa

#endif  // HOTAOSA_STRING_RLE_H_
