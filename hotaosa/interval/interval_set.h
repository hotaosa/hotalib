#ifndef HOTAOSA_INTERVAL_INTERVAL_SET_H_
#define HOTAOSA_INTERVAL_INTERVAL_SET_H_

#include <cassert>
#include <concepts>
#include <functional>
#include <limits>
#include <type_traits>

#include <boost/icl/interval.hpp>
#include <boost/icl/interval_set.hpp>

namespace hotaosa {

// IntervalSet offers a thin, contest-friendly facade over
// boost::icl::interval_set for integral domains. The right-open [lower, upper)
// interval is the default primitive, matching typical AtCoder-style half-open
// ranges. Most operations run in O(log M), where M is the number of disjoint
// intervals tracked by the set. For signed Key types, public APIs assert that
// arguments are non-negative.
template <std::integral Key, template <class> class Compare = std::less>
class IntervalSet {
 public:
  using Impl = boost::icl::interval_set<Key, Compare>;
  using Interval = typename Impl::interval_type;
  using iterator = typename Impl::const_iterator;
  using size_type = typename Impl::size_type;
  using CompareType = Compare<Key>;
  using value_type = Key;

  IntervalSet() = default;

  IntervalSet(const IntervalSet&) = default;
  IntervalSet& operator=(const IntervalSet&) = default;
  IntervalSet(IntervalSet&&) = default;
  IntervalSet& operator=(IntervalSet&&) = default;
  ~IntervalSet() = default;

  // Adds [lower, upper) to the set. O(log M).
  void Add(const Key& lower, const Key& upper) {
    const CompareType comp{};
    CheckNonNegative(lower);
    CheckNonNegative(upper);
    assert(!comp(upper, lower));
    if (!comp(lower, upper)) {
      return;
    }
    impl_.add(RightOpen(lower, upper));
  }

  // Adds single `value`, interpreted as [value, value + 1). O(log M).
  void Add(const Key& value) {
    CheckNonNegative(value);
    impl_.add(RightOpen(value));
  }

  // Replaces the current contents with exactly [lower, upper). O(log M).
  void Assign(const Key& lower, const Key& upper) {
    CheckNonNegative(lower);
    CheckNonNegative(upper);
    impl_.clear();
    Add(lower, upper);
  }

  // Replaces the current contents with the single element `value`. O(log M).
  void Assign(const Key& value) {
    CheckNonNegative(value);
    impl_.clear();
    Add(value);
  }

  // Removes [lower, upper) when present. O(log M).
  void Erase(const Key& lower, const Key& upper) {
    const CompareType comp{};
    CheckNonNegative(lower);
    CheckNonNegative(upper);
    assert(!comp(upper, lower));
    if (!comp(lower, upper)) {
      return;
    }
    impl_.subtract(RightOpen(lower, upper));
  }

  // Removes single `value`, interpreted as [value, value + 1). O(log M).
  void Erase(const Key& value) {
    CheckNonNegative(value);
    impl_.subtract(RightOpen(value));
  }

  // Returns whether `value` is contained in any stored interval. O(log M).
  [[nodiscard]] bool Contains(const Key& value) const {
    CheckNonNegative(value);
    return impl_.find(value) != impl_.end();
  }

  // Returns the smallest non-negative value that is not covered; O(1).
  [[nodiscard]] Key Mex() const {
    if (impl_.empty()) {
      return static_cast<Key>(0);
    }
    const auto first = impl_.begin();
    return first->lower() == static_cast<Key>(0) ? first->upper()
                                                 : static_cast<Key>(0);
  }

  // Returns the smallest value >= start that is not covered. O(log M).
  [[nodiscard]] Key Mex(Key start) const {
    const CompareType comp{};
    if constexpr (std::is_signed_v<Key>) {
      if (start < static_cast<Key>(0)) {
        start = static_cast<Key>(0);
      }
    }
    const auto covering = impl_.find(start);
    if (covering != impl_.end()) {
      return covering->upper();
    }
    const Key kMax = std::numeric_limits<Key>::max();
    if (start == kMax) {
      return start;
    }
    const auto next = impl_.lower_bound(RightOpen(start, NextValue(start)));
    if (next == impl_.end()) {
      return start;
    }
    const Key lower = next->lower();
    if (comp(start, lower)) {
      return start;
    }
    return next->upper();
  }

  // Returns whether the whole [lower, upper) range is covered. O(log M).
  [[nodiscard]] bool Covers(const Key& lower, const Key& upper) const {
    const CompareType comp{};
    assert(!comp(upper, lower));
    if (!comp(lower, upper)) {
      return true;
    }
    const auto it = impl_.find(lower);
    if (it == impl_.end()) {
      return false;
    }
    return !comp(it->upper(), upper);
  }

  // Returns true when no interval is stored. O(1).
  [[nodiscard]] bool Empty() const { return impl_.empty(); }

  // Returns the total cardinality/measure; O(M) over disjoint segments.
  [[nodiscard]] size_type Size() const { return impl_.size(); }

  [[nodiscard]] iterator begin() const { return impl_.begin(); }
  [[nodiscard]] iterator end() const { return impl_.end(); }

  [[nodiscard]] const Impl& impl() const { return impl_; }
  [[nodiscard]] Impl& impl() { return impl_; }

 private:
  static void CheckNonNegative(const Key& value) {
    if constexpr (std::is_signed_v<Key>) {
      assert(value >= static_cast<Key>(0));
    }
  }

  static Interval RightOpen(const Key& lower, const Key& upper) {
    return boost::icl::interval<Key>::right_open(lower, upper);
  }

  static Interval RightOpen(const Key& value) {
    return RightOpen(value, NextValue(value));
  }

  static Key NextValue(const Key& value) {
    [[maybe_unused]] constexpr Key kMax = std::numeric_limits<Key>::max();
    if constexpr (std::is_signed_v<Key>) {
      assert(value < kMax);
    } else {
      assert(value != kMax);
    }
    return static_cast<Key>(value + static_cast<Key>(1));
  }

  Impl impl_;
};

}  // namespace hotaosa

#endif  // HOTAOSA_INTERVAL_INTERVAL_SET_H_
