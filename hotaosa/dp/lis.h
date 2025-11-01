#ifndef HOTAOSA_DP_LIS_H_
#define HOTAOSA_DP_LIS_H_

#include <concepts>
#include <cstdint>
#include <functional>
#include <ranges>
#include <utility>
#include <vector>

namespace hotaosa {

enum class LisMode : std::uint8_t {
  kStrict,
  kNonStrict,
};

namespace internal {

template <typename Value, typename Compare>
int LisFindInsertPos(const std::vector<Value>& tails,
                     const Value& value,
                     Compare& comp,
                     LisMode mode) {
  int left = 0;
  int right = static_cast<int>(tails.size());
  while (left < right) {
    const int mid = (left + right) / 2;
    if (mode == LisMode::kStrict) {
      if (comp(tails[mid], value)) {
        left = mid + 1;
      } else {
        right = mid;
      }
    } else {
      if (!comp(value, tails[mid])) {
        left = mid + 1;
      } else {
        right = mid;
      }
    }
  }
  return left;
}

template <typename Compare>
class ReverseComparator {
 public:
  ReverseComparator() = default;
  explicit ReverseComparator(Compare comp) : comp_(std::move(comp)) {}

  template <typename Lhs, typename Rhs>
  constexpr bool operator()(Lhs&& lhs, Rhs&& rhs) {
    return comp_(std::forward<Rhs>(rhs), std::forward<Lhs>(lhs));
  }

 private:
  Compare comp_;
};

template <std::ranges::random_access_range Range, typename Compare>
auto LisComputeIndices(const Range& range, Compare& comp, LisMode mode)
    -> std::vector<int>
  requires std::copy_constructible<std::ranges::range_value_t<Range>> &&
           std::indirect_strict_weak_order<Compare&,
                                           std::ranges::iterator_t<Range>>
{
  const int n = static_cast<int>(std::ranges::distance(range));
  if (n == 0) {
    return {};
  }
  using Value = std::ranges::range_value_t<Range>;
  std::vector<Value> tails;
  std::vector<int> tail_indices;
  std::vector<int> parent(n, -1);
  tails.reserve(n);
  tail_indices.reserve(n);
  auto first = std::ranges::begin(range);
  for (int i = 0; i < n; ++i) {
    const Value& element = first[i];
    const int pos = LisFindInsertPos(tails, element, comp, mode);
    if (pos == static_cast<int>(tails.size())) {
      tails.push_back(element);
      tail_indices.push_back(i);
    } else {
      tails[pos] = element;
      tail_indices[pos] = i;
    }
    if (pos > 0) {
      parent[i] = tail_indices[pos - 1];
    }
  }
  std::vector<int> indices(tail_indices.size());
  int index = tail_indices.empty() ? -1 : tail_indices.back();
  for (int pos = static_cast<int>(indices.size()) - 1; pos >= 0; --pos) {
    indices[pos] = index;
    index = parent[index];
  }
  return indices;
}

}  // namespace internal

// Returns LIS lengths for every index when treated as the subsequence end.
// Complexity: O(N log N), where N is the length of `range`.
template <std::ranges::random_access_range Range,
          typename Compare = std::ranges::less>
auto LisEndingLengths(const Range& range,
                      Compare comp = Compare{},
                      LisMode mode = LisMode::kStrict) -> std::vector<int>
  requires std::copy_constructible<std::ranges::range_value_t<Range>> &&
           std::indirect_strict_weak_order<Compare&,
                                           std::ranges::iterator_t<Range>>
{
  const int n = static_cast<int>(std::ranges::distance(range));
  std::vector<int> lengths(n);
  std::vector<std::ranges::range_value_t<Range>> tails;
  tails.reserve(n);
  auto first = std::ranges::begin(range);
  for (int i = 0; i < n; ++i) {
    const auto& element = first[i];
    const int pos = internal::LisFindInsertPos(tails, element, comp, mode);
    if (pos == static_cast<int>(tails.size())) {
      tails.push_back(element);
    } else {
      tails[pos] = element;
    }
    lengths[i] = pos + 1;
  }
  return lengths;
}

// Returns LIS lengths for every index when treated as the subsequence start.
// Complexity: O(N log N), where N is the length of `range`.
template <std::ranges::random_access_range Range,
          typename Compare = std::ranges::less>
auto LisStartingLengths(const Range& range,
                        Compare comp = Compare{},
                        LisMode mode = LisMode::kStrict) -> std::vector<int>
  requires std::copy_constructible<std::ranges::range_value_t<Range>> &&
           std::indirect_strict_weak_order<Compare&,
                                           std::ranges::iterator_t<Range>>
{
  if (std::ranges::empty(range)) {
    return {};
  }
  auto reversed = range | std::views::reverse;
  auto reversed_comp = internal::ReverseComparator<Compare>(std::move(comp));
  auto reversed_lengths =
      LisEndingLengths(reversed, std::move(reversed_comp), mode);
  return std::vector<int>(reversed_lengths.rbegin(), reversed_lengths.rend());
}

// Returns one longest increasing subsequence as indices in `range`.
// Complexity: O(N log N), where N is the length of `range`.
template <std::ranges::random_access_range Range,
          typename Compare = std::ranges::less>
auto LisIndices(const Range& range,
                Compare comp = Compare{},
                LisMode mode = LisMode::kStrict) -> std::vector<int>
  requires std::copy_constructible<std::ranges::range_value_t<Range>> &&
           std::indirect_strict_weak_order<Compare&,
                                           std::ranges::iterator_t<Range>>
{
  return internal::LisComputeIndices(range, comp, mode);
}

// Returns the length of a longest increasing subsequence in `range`.
// Complexity: O(N log N), where N is the length of `range`.
template <std::ranges::forward_range Range,
          typename Compare = std::ranges::less>
int LisLength(const Range& range,
              Compare comp = Compare{},
              LisMode mode = LisMode::kStrict)
  requires std::copy_constructible<std::ranges::range_value_t<Range>> &&
           std::indirect_strict_weak_order<Compare&,
                                           std::ranges::iterator_t<Range>>
{
  using Value = std::ranges::range_value_t<Range>;
  std::vector<Value> tails;
  if constexpr (std::ranges::sized_range<Range>) {
    tails.reserve(std::ranges::size(range));
  }
  for (const auto& element : range) {
    const int pos = internal::LisFindInsertPos(tails, element, comp, mode);
    if (pos == static_cast<int>(tails.size())) {
      tails.push_back(element);
    } else {
      tails[pos] = element;
    }
  }
  return static_cast<int>(tails.size());
}

// Returns one longest increasing subsequence of `range`.
// Complexity: O(N log N), where N is the length of `range`.
template <std::ranges::random_access_range Range,
          typename Compare = std::ranges::less>
auto Lis(const Range& range,
         Compare comp = Compare{},
         LisMode mode = LisMode::kStrict)
    -> std::vector<std::ranges::range_value_t<Range>>
  requires std::copy_constructible<std::ranges::range_value_t<Range>> &&
           std::indirect_strict_weak_order<Compare&,
                                           std::ranges::iterator_t<Range>>
{
  auto indices = internal::LisComputeIndices(range, comp, mode);
  std::vector<std::ranges::range_value_t<Range>> sequence(indices.size());
  auto first = std::ranges::begin(range);
  for (int i = 0; i < static_cast<int>(indices.size()); ++i) {
    sequence[i] = first[indices[i]];
  }
  return sequence;
}

}  // namespace hotaosa

#endif  // HOTAOSA_DP_LIS_H_
