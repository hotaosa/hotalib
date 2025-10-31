#ifndef HOTAOSA_DS_TRIE_H_
#define HOTAOSA_DS_TRIE_H_

#include <algorithm>
#include <array>
#include <cassert>
#include <memory>
#include <string_view>
#include <type_traits>

namespace hotaosa {

template <int kNumChar, char kBase, typename CountType = int>
class Trie {
 public:
  static_assert(std::is_integral_v<CountType>,
                "Trie count type must be an integral type");
  // Characters must map to indices in [0, kNumChar); violations trigger debug
  // asserts. All operations below run in O(|word|).
  Trie(const Trie&) = delete;
  Trie& operator=(const Trie&) = delete;
  Trie(Trie&&) = delete;
  Trie& operator=(Trie&&) = delete;

  Trie() : root_(std::make_unique<Node>(nullptr)) {}

  // ----- Mutating operations -----

  // Convenience wrapper: insert one copy of `word`. O(|word|).
  void Insert(std::string_view word) {
    Insert(word, static_cast<CountType>(1));
  }

  // Inserts `word` and increases its count by `count`. O(|word|).
  void Insert(std::string_view word, CountType count) {
    assert(count >= 0);
    if (count <= 0) {
      return;
    }
    Node* node = root_.get();
    node->prefix_count += count;
    for (const char ch : word) {
      const int idx = ch - kBase;
      assert(IsValidIndex(idx));
      if (!node->children[idx]) {
        node->children[idx] = std::make_unique<Node>(node);
      }
      node = node->children[idx].get();
      node->prefix_count += count;
    }
    node->end_count += count;
  }

  // Removes one occurrence if present. O(|word|).
  void Remove(std::string_view word) {
    Remove(word, static_cast<CountType>(1));
  }

  // Decrements the count by `count` (clamped at zero). O(|word|).
  void Remove(std::string_view word, CountType count) {
    assert(count >= 0);
    if (count <= 0) {
      return;
    }
    Node* node = Find(word);
    if (node == nullptr) {
      return;
    }
    const CountType decrement = std::min(count, node->end_count);
    if (decrement <= 0) {
      return;
    }
    node->end_count -= decrement;
    SubtractFromAncestors(node, decrement);
  }

  // Removes every string that has `prefix` as a prefix. O(|prefix|).
  void RemoveWithPrefix(std::string_view prefix) {
    Node* node = Find(prefix);
    if (node == nullptr) {
      return;
    }
    const CountType total = node->prefix_count;
    if (total <= 0) {
      return;
    }
    if (node->parent == nullptr) {
      node->end_count = 0;
      node->prefix_count = 0;
      for (auto& child : node->children) {
        child.reset();
      }
      return;
    }
    Node* parent = node->parent;
    const char last = prefix.back();
    const int idx = last - kBase;
    assert(IsValidIndex(idx));
    parent->children[idx].reset();
    SubtractFromAncestors(parent, total);
  }

  // Removes every stored string that is a prefix of `word`. O(|word|).
  void RemovePrefixesOf(std::string_view word) {
    Node* node = root_.get();
    if (node == nullptr) {
      return;
    }
    if (node->end_count > 0) {
      const CountType dec = node->end_count;
      node->end_count = 0;
      SubtractFromAncestors(node, dec);
    }
    for (const char ch : word) {
      const int idx = ch - kBase;
      if (!IsValidIndex(idx) || !node->children[idx]) {
        break;
      }
      node = node->children[idx].get();
      if (node->end_count > 0) {
        const CountType dec = node->end_count;
        node->end_count = 0;
        SubtractFromAncestors(node, dec);
      }
    }
  }

  // ----- Aggregate queries -----

  // Total multiplicity of strings stored in the trie. O(1).
  [[nodiscard]] CountType TotalCount() const {
    return root_ == nullptr ? static_cast<CountType>(0) : root_->prefix_count;
  }

  // Returns the multiplicity of `word`. O(|word|).
  [[nodiscard]] CountType Count(std::string_view word) const {
    const Node* node = Find(word);
    return node == nullptr ? static_cast<CountType>(0) : node->end_count;
  }

  // Returns the total multiplicity of strings that have `prefix` as a prefix.
  // O(|prefix|).
  [[nodiscard]] CountType CountWithPrefix(std::string_view prefix) const {
    const Node* node = Find(prefix);
    return node == nullptr ? static_cast<CountType>(0) : node->prefix_count;
  }

  // Returns how many stored strings are prefixes of `word`. O(|word|).
  [[nodiscard]] CountType CountPrefixesOf(std::string_view word) const {
    const Node* node = root_.get();
    CountType total = node->end_count;
    for (const char ch : word) {
      const int idx = ch - kBase;
      if (!IsValidIndex(idx) || !node->children[idx]) {
        break;
      }
      node = node->children[idx].get();
      total += node->end_count;
    }
    return total;
  }

  // ----- Boolean queries -----

  // True if `word` exists with positive count. O(|word|).
  [[nodiscard]] bool Contains(std::string_view word) const {
    return Count(word) > 0;
  }

  // True if there exists a stored string that has `prefix` as a prefix.
  // O(|prefix|).
  [[nodiscard]] bool ContainsWithPrefix(std::string_view prefix) const {
    return CountWithPrefix(prefix) > 0;
  }

  // True if some stored string is a prefix of `word`. O(|word|).
  [[nodiscard]] bool ContainsPrefixOf(std::string_view word) const {
    return CountPrefixesOf(word) > 0;
  }

  // ----- Miscellaneous -----

  // Length of the longest common prefix with any stored string. O(|word|).
  [[nodiscard]] int LcpWith(std::string_view word) const {
    const Node* p = root_.get();
    const int len = static_cast<int>(word.size());
    for (int i = 0; i < len; ++i) {
      const int idx = word[i] - kBase;
      if (!IsValidIndex(idx) || !p->children[idx]) {
        return i;
      }
      p = p->children[idx].get();
    }
    return len;
  }

 private:
  struct Node {
    CountType prefix_count{0};
    CountType end_count{0};
    Node* parent{nullptr};
    std::array<std::unique_ptr<Node>, kNumChar> children{};

    explicit Node(Node* parent) : parent(parent) {}
  };

  std::unique_ptr<Node> root_;

  static constexpr bool IsValidIndex(int idx) {
    return 0 <= idx && idx < kNumChar;
  }

  // Returns a mutable pointer to the node representing `word`, or null if the
  // path does not exist.
  Node* Find(std::string_view word) {
    Node* p = root_.get();
    for (const char ch : word) {
      const int idx = ch - kBase;
      if (!IsValidIndex(idx) || !p->children[idx]) {
        return nullptr;
      }
      p = p->children[idx].get();
    }
    return p;
  }

  // Const-qualified overload returning a read-only pointer to the node for
  // `word`.
  const Node* Find(std::string_view word) const {
    return const_cast<Trie*>(this)->Find(word);
  }

  static void SubtractFromAncestors(Node* node, CountType dec) {
    while (node != nullptr) {
      if (node->prefix_count > dec) {
        node->prefix_count -= dec;
      } else {
        node->prefix_count = 0;
      }
      node = node->parent;
    }
  }
};

}  // namespace hotaosa

#endif  // HOTAOSA_DS_TRIE_H_
