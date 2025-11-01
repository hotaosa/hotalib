#ifndef HOTAOSA_DS_BINARY_TRIE_H_
#define HOTAOSA_DS_BINARY_TRIE_H_

#include <algorithm>
#include <array>
#include <cassert>
#include <concepts>
#include <limits>
#include <optional>
#include <type_traits>
#include <vector>

namespace hotaosa {

// BinaryTrie stores unsigned integers (multiset semantics) in O(kNumBits) per
// operation. The trie is parameterised by ValueType and the number of tracked
// bits, and supports a lazy XOR mask for whole-set toggling.
template <std::unsigned_integral ValueType,
          int kNumBits = std::numeric_limits<ValueType>::digits,
          std::integral CountType = int>
class BinaryTrie {
  static_assert(kNumBits > 0, "BinaryTrie requires at least one bit");
  static_assert(kNumBits <= std::numeric_limits<ValueType>::digits,
                "BinaryTrie bit width exceeds ValueType digits");

 public:
  BinaryTrie() : nodes_(1) {}

  BinaryTrie(const BinaryTrie&) = delete;
  BinaryTrie& operator=(const BinaryTrie&) = delete;
  BinaryTrie(BinaryTrie&&) = delete;
  BinaryTrie& operator=(BinaryTrie&&) = delete;

  // Inserts one copy of `value`. O(kNumBits).
  void Insert(ValueType value) { Insert(value, static_cast<CountType>(1)); }

  // Inserts `count` copies of `value`. O(kNumBits).
  void Insert(ValueType value, CountType count) {
    assert(count >= 0);
    if (count == 0) {
      return;
    }
    const ValueType mask = BitMask();
    assert((value & ~mask) == 0);
    const ValueType stored_value = ToStored(value);
    int node_index = 0;
    nodes_[node_index].subtree_count += count;
    for (int bit = kNumBits - 1; bit >= 0; --bit) {
      const int direction = static_cast<int>((stored_value >> bit) & 1);
      int child_index = nodes_[node_index].children[direction];
      if (child_index == kNull) {
        child_index = NewNode();
        nodes_[node_index].children[direction] = child_index;
      }
      node_index = child_index;
      nodes_[node_index].subtree_count += count;
    }
    nodes_[node_index].terminal_count += count;
  }

  // Removes one copy of `value` when present. O(kNumBits).
  void Erase(ValueType value) { Erase(value, static_cast<CountType>(1)); }

  // Removes up to `count` copies of `value`. O(kNumBits).
  void Erase(ValueType value, CountType count) {
    assert(count >= 0);
    if (count == 0) {
      return;
    }
    const ValueType mask = BitMask();
    assert((value & ~mask) == 0);
    const ValueType stored_value = ToStored(value);
    std::array<int, kNumBits + 1> path{};
    int node_index = 0;
    path[0] = node_index;
    for (int bit = kNumBits - 1; bit >= 0; --bit) {
      const int direction = static_cast<int>((stored_value >> bit) & 1);
      const int child_index = nodes_[node_index].children[direction];
      if (child_index == kNull) {
        return;
      }
      node_index = child_index;
      path[kNumBits - bit] = node_index;
    }
    const CountType removable =
        std::min(count, nodes_[node_index].terminal_count);
    if (removable == 0) {
      return;
    }
    nodes_[node_index].terminal_count -= removable;
    for (int depth = kNumBits; depth >= 0; --depth) {
      nodes_[path[depth]].subtree_count -= removable;
    }
  }

  // Returns the multiplicity of `value` stored in the trie. O(kNumBits).
  [[nodiscard]] CountType Count(ValueType value) const {
    const ValueType mask = BitMask();
    assert((value & ~mask) == 0);
    const ValueType stored_value = ToStored(value);
    int node_index = 0;
    for (int bit = kNumBits - 1; bit >= 0; --bit) {
      const int direction = static_cast<int>((stored_value >> bit) & 1);
      const int child_index = nodes_[node_index].children[direction];
      if (child_index == kNull) {
        return static_cast<CountType>(0);
      }
      node_index = child_index;
    }
    return nodes_[node_index].terminal_count;
  }

  // Total multiplicity stored in the trie. O(1).
  [[nodiscard]] CountType TotalCount() const { return nodes_[0].subtree_count; }

  // Returns whether the multiset currently contains `value`. O(kNumBits).
  [[nodiscard]] bool Contains(ValueType value) const {
    return Count(value) > static_cast<CountType>(0);
  }

  // Returns how many stored values are strictly less than `value`. O(kNumBits).
  [[nodiscard]] CountType CountLess(ValueType value) const {
    const ValueType mask = BitMask();
    assert((value & ~mask) == 0);
    CountType result = 0;
    int node_index = 0;
    for (int bit = kNumBits - 1; bit >= 0 && node_index != kNull; --bit) {
      const int mask_bit = static_cast<int>((xor_mask_ >> bit) & 1);
      const int zero_child = nodes_[node_index].children[mask_bit];
      const int one_child = nodes_[node_index].children[mask_bit ^ 1];
      const int actual_bit = static_cast<int>((value >> bit) & 1);
      if (actual_bit == 1) {
        result += SubtreeCount(zero_child);
        node_index = one_child;
      } else {
        node_index = zero_child;
      }
    }
    return result;
  }

  // Returns how many stored values are strictly greater than `value`.
  // O(kNumBits).
  [[nodiscard]] CountType CountGreater(ValueType value) const {
    const CountType less = CountLess(value);
    const CountType equal = Count(value);
    return static_cast<CountType>(TotalCount() - less - equal);
  }

  // Returns the k-th smallest value (0-indexed). O(kNumBits).
  [[nodiscard]] std::optional<ValueType> Kth(CountType k) const {
    if (k < 0) {
      return std::nullopt;
    }
    const CountType total = TotalCount();
    if (total <= 0) {
      return std::nullopt;
    }
    using UnsignedCount = std::make_unsigned_t<CountType>;
    const auto target = static_cast<UnsignedCount>(k);
    const auto total_unsigned = static_cast<UnsignedCount>(total);
    if (target >= total_unsigned) {
      return std::nullopt;
    }
    int node_index = 0;
    ValueType stored_value = 0;
    UnsignedCount remaining = target;
    for (int bit = kNumBits - 1; bit >= 0; --bit) {
      const int mask_bit = static_cast<int>((xor_mask_ >> bit) & 1);
      const int zero_child = nodes_[node_index].children[mask_bit];
      const auto zero_count =
          static_cast<UnsignedCount>(SubtreeCount(zero_child));
      if (remaining < zero_count) {
        node_index = zero_child;
        if (mask_bit == 1) {
          stored_value |= (ValueType{1} << bit);
        }
        continue;
      }
      remaining -= zero_count;
      const int one_child = nodes_[node_index].children[mask_bit ^ 1];
      if (one_child == kNull || SubtreeCount(one_child) <= 0) {
        return std::nullopt;
      }
      node_index = one_child;
      if ((mask_bit ^ 1) == 1) {
        stored_value |= (ValueType{1} << bit);
      }
    }
    return ToActual(stored_value);
  }

  // Returns minimal value >= `value`. O(kNumBits).
  [[nodiscard]] std::optional<ValueType> LowerBound(ValueType value) const {
    if (TotalCount() <= 0) {
      return std::nullopt;
    }
    const ValueType mask = BitMask();
    assert((value & ~mask) == 0);
    ValueType result = 0;
    if (!FindLowerBound(0, kNumBits - 1, value & mask, 0, true, &result)) {
      return std::nullopt;
    }
    return result & mask;
  }

  // Returns maximal value <= `value`. O(kNumBits).
  [[nodiscard]] std::optional<ValueType> Prev(ValueType value) const {
    if (TotalCount() <= 0) {
      return std::nullopt;
    }
    const ValueType mask = BitMask();
    assert((value & ~mask) == 0);
    ValueType result = 0;
    if (!FindPrev(0, kNumBits - 1, value & mask, 0, true, &result)) {
      return std::nullopt;
    }
    return result & mask;
  }

  // Returns the maximum value of (element XOR `value`). O(kNumBits).
  [[nodiscard]] std::optional<ValueType> MaxXor(ValueType value) const {
    assert((value & ~BitMask()) == 0);
    ValueType stored = 0;
    if (!FindExtremeXor(value & BitMask(), true, &stored)) {
      return std::nullopt;
    }
    return (ToActual(stored) ^ value) & BitMask();
  }

  // Returns the minimum value of (element XOR `value`). O(kNumBits).
  [[nodiscard]] std::optional<ValueType> MinXor(ValueType value) const {
    assert((value & ~BitMask()) == 0);
    ValueType stored = 0;
    if (!FindExtremeXor(value & BitMask(), false, &stored)) {
      return std::nullopt;
    }
    return (ToActual(stored) ^ value) & BitMask();
  }

  // Applies XOR with `mask` lazily to every stored value. O(1).
  void XorAll(ValueType mask) { xor_mask_ ^= (mask & BitMask()); }

 private:
  static constexpr int kNull = -1;

  struct Node {
    std::array<int, 2> children{{kNull, kNull}};
    CountType subtree_count{0};
    CountType terminal_count{0};
  };

  [[nodiscard]] static constexpr ValueType BitMask() {
    if constexpr (kNumBits >= std::numeric_limits<ValueType>::digits) {
      return std::numeric_limits<ValueType>::max();
    } else {
      return (ValueType{1} << kNumBits) - ValueType{1};
    }
  }

  [[nodiscard]] ValueType ToStored(ValueType value) const {
    return (value ^ xor_mask_) & BitMask();
  }

  [[nodiscard]] ValueType ToActual(ValueType stored) const {
    return (stored ^ xor_mask_) & BitMask();
  }

  [[nodiscard]] CountType SubtreeCount(int node_index) const {
    return node_index == kNull ? static_cast<CountType>(0)
                               : nodes_[node_index].subtree_count;
  }

  [[nodiscard]] int ChildForActualBit(int node_index,
                                      int bit,
                                      int actual_bit) const {
    const int stored_bit =
        actual_bit ^ static_cast<int>((xor_mask_ >> bit) & 1);
    return nodes_[node_index].children[stored_bit];
  }

  bool FindLowerBound(int node_index,
                      int bit,
                      ValueType target,
                      ValueType prefix_actual,
                      bool tight,
                      ValueType* result) const {
    if (node_index == kNull || SubtreeCount(node_index) <= 0) {
      return false;
    }
    if (bit < 0) {
      if (nodes_[node_index].terminal_count > 0) {
        *result = prefix_actual;
        return true;
      }
      return false;
    }
    const int target_bit = static_cast<int>((target >> bit) & 1);
    if (tight) {
      const int child_same = ChildForActualBit(node_index, bit, target_bit);
      if (child_same != kNull &&
          FindLowerBound(
              child_same,
              bit - 1,
              target,
              static_cast<ValueType>(
                  prefix_actual | (static_cast<ValueType>(target_bit) << bit)),
              true,
              result)) {
        return true;
      }
      if (target_bit == 0) {
        const int child_one = ChildForActualBit(node_index, bit, 1);
        if (child_one != kNull &&
            FindLowerBound(
                child_one,
                bit - 1,
                target,
                static_cast<ValueType>(prefix_actual | (ValueType{1} << bit)),
                false,
                result)) {
          return true;
        }
      }
      return false;
    }
    for (int actual_bit = 0; actual_bit <= 1; ++actual_bit) {
      const int child = ChildForActualBit(node_index, bit, actual_bit);
      if (child != kNull &&
          FindLowerBound(
              child,
              bit - 1,
              target,
              static_cast<ValueType>(
                  prefix_actual | (static_cast<ValueType>(actual_bit) << bit)),
              false,
              result)) {
        return true;
      }
    }
    return false;
  }

  bool FindPrev(int node_index,
                int bit,
                ValueType target,
                ValueType prefix_actual,
                bool tight,
                ValueType* result) const {
    if (node_index == kNull || SubtreeCount(node_index) <= 0) {
      return false;
    }
    if (bit < 0) {
      if (nodes_[node_index].terminal_count > 0) {
        *result = prefix_actual;
        return true;
      }
      return false;
    }
    const int target_bit = static_cast<int>((target >> bit) & 1);
    if (tight) {
      const int child_same = ChildForActualBit(node_index, bit, target_bit);
      if (child_same != kNull &&
          FindPrev(
              child_same,
              bit - 1,
              target,
              static_cast<ValueType>(
                  prefix_actual | (static_cast<ValueType>(target_bit) << bit)),
              true,
              result)) {
        return true;
      }
      if (target_bit == 1) {
        const int child_zero = ChildForActualBit(node_index, bit, 0);
        if (child_zero != kNull &&
            FindPrev(
                child_zero, bit - 1, target, prefix_actual, false, result)) {
          return true;
        }
      }
      return false;
    }
    for (int actual_bit = 1; actual_bit >= 0; --actual_bit) {
      const int child = ChildForActualBit(node_index, bit, actual_bit);
      if (child != kNull &&
          FindPrev(
              child,
              bit - 1,
              target,
              static_cast<ValueType>(
                  prefix_actual | (static_cast<ValueType>(actual_bit) << bit)),
              false,
              result)) {
        return true;
      }
    }
    return false;
  }

  bool FindExtremeXor(ValueType value,
                      bool maximize,
                      ValueType* stored_value) const {
    if (TotalCount() <= 0) {
      return false;
    }
    const ValueType mask = BitMask();
    ValueType key = (value ^ xor_mask_) & mask;
    int node_index = 0;
    ValueType result = 0;
    for (int bit = kNumBits - 1; bit >= 0; --bit) {
      int desired =
          static_cast<int>(((key >> bit) & 1) ^ static_cast<int>(maximize));
      int child = nodes_[node_index].children[desired];
      if (child == kNull || SubtreeCount(child) <= 0) {
        desired ^= 1;
        child = nodes_[node_index].children[desired];
        if (child == kNull || SubtreeCount(child) <= 0) {
          return false;
        }
      }
      if (desired == 1) {
        result |= (ValueType{1} << bit);
      }
      node_index = child;
    }
    *stored_value = result;
    return true;
  }

  int NewNode() {
    nodes_.emplace_back();
    return static_cast<int>(nodes_.size() - 1);
  }

  std::vector<Node> nodes_;
  ValueType xor_mask_{0};
};

}  // namespace hotaosa

#endif  // HOTAOSA_DS_BINARY_TRIE_H_
