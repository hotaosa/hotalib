#ifndef HOTAOSA_DS_TRIE_H_
#define HOTAOSA_DS_TRIE_H_

#include <algorithm>
#include <array>
#include <cassert>
#include <concepts>
#include <string_view>
#include <vector>

namespace hotaosa {

// Generic trie over a fixed alphabet [kBase, kBase + kNumChar).
// Stores multiplicities of strings and supports O(|word|) updates/queries.
template <int kNumChar, char kBase, std::integral CountType = int>
class Trie {
  static_assert(kNumChar > 0, "Trie requires a positive alphabet size");

 public:
  Trie() : nodes_(1) {}

  Trie(const Trie&) = delete;
  Trie& operator=(const Trie&) = delete;
  Trie(Trie&&) = delete;
  Trie& operator=(Trie&&) = delete;

  // Inserts one copy of `word`. O(|word|).
  void Insert(std::string_view word) {
    Insert(word, static_cast<CountType>(1));
  }

  // Inserts `count` copies of `word`. O(|word|).
  void Insert(std::string_view word, CountType count) {
    assert(count >= 0);
    if (count <= 0) {
      return;
    }
    int node_index = 0;
    nodes_[node_index].prefix_count += count;
    for (const char ch : word) {
      const int idx = ch - kBase;
      assert(IsValidIndex(idx));
      int child_index = nodes_[node_index].children[idx];
      if (child_index == kNull) {
        child_index = NewNode();
        nodes_[node_index].children[idx] = child_index;
      }
      node_index = child_index;
      nodes_[node_index].prefix_count += count;
    }
    nodes_[node_index].end_count += count;
  }

  // Removes one copy of `word` when present. O(|word|).
  void Remove(std::string_view word) {
    Remove(word, static_cast<CountType>(1));
  }

  // Removes up to `count` copies of `word`. O(|word|).
  void Remove(std::string_view word, CountType count) {
    assert(count >= 0);
    if (count <= 0) {
      return;
    }
    std::vector<int> path;
    const int node_index = FindNode(word, &path);
    if (node_index == kNull) {
      return;
    }
    const CountType removable =
        std::min(count, nodes_[node_index].end_count);
    if (removable <= 0) {
      return;
    }
    nodes_[node_index].end_count -= removable;
    SubtractAlongPath(path, removable);
  }

  // Removes every string that has `prefix` as a prefix. O(|prefix| + size of subtree).
  void RemoveWithPrefix(std::string_view prefix) {
    std::vector<int> path;
    const int node_index = FindNode(prefix, &path);
    if (node_index == kNull) {
      return;
    }
    const CountType total = nodes_[node_index].prefix_count;
    if (total <= 0) {
      return;
    }
    if (path.size() == 1) {
      ClearSubtree(node_index);
      return;
    }
    path.pop_back();  // retain ancestors only
    SubtractAlongPath(path, total);
    const int parent_index = path.back();
    const char last = prefix.back();
    const int idx = last - kBase;
    assert(IsValidIndex(idx));
    nodes_[parent_index].children[idx] = kNull;
    ClearSubtree(node_index);
  }

  // Removes every stored string that is a prefix of `word`. O(|word|).
  void RemovePrefixesOf(std::string_view word) {
    std::vector<int> path;
    path.reserve(word.size() + 1);
    int node_index = 0;
    path.push_back(node_index);
    if (nodes_[node_index].end_count > 0) {
      const CountType dec = nodes_[node_index].end_count;
      nodes_[node_index].end_count = 0;
      SubtractAlongPath(path, dec);
    }
    for (const char ch : word) {
      const int idx = ch - kBase;
      if (!IsValidIndex(idx)) {
        return;
      }
      const int child_index = nodes_[node_index].children[idx];
      if (child_index == kNull) {
        return;
      }
      node_index = child_index;
      path.push_back(node_index);
      if (nodes_[node_index].end_count > 0) {
        const CountType dec = nodes_[node_index].end_count;
        nodes_[node_index].end_count = 0;
        SubtractAlongPath(path, dec);
      }
    }
  }

  // ----- Aggregate queries -----

  // Total multiplicity of stored strings. O(1).
  [[nodiscard]] CountType TotalCount() const {
    return nodes_[0].prefix_count;
  }

  // Multiplicity of `word`. O(|word|).
  [[nodiscard]] CountType Count(std::string_view word) const {
    const int node_index = FindNode(word);
    return node_index == kNull ? static_cast<CountType>(0)
                               : nodes_[node_index].end_count;
  }

  // Total multiplicity of strings with `prefix` as a prefix. O(|prefix|).
  [[nodiscard]] CountType CountWithPrefix(std::string_view prefix) const {
    const int node_index = FindNode(prefix);
    return node_index == kNull ? static_cast<CountType>(0)
                               : nodes_[node_index].prefix_count;
  }

  // Number of stored strings that are prefixes of `word`. O(|word|).
  [[nodiscard]] CountType CountPrefixesOf(std::string_view word) const {
    int node_index = 0;
    CountType total = nodes_[node_index].end_count;
    for (const char ch : word) {
      const int idx = ch - kBase;
      if (!IsValidIndex(idx)) {
        break;
      }
      const int child_index = nodes_[node_index].children[idx];
      if (child_index == kNull) {
        break;
      }
      node_index = child_index;
      total += nodes_[node_index].end_count;
    }
    return total;
  }

  // ----- Boolean queries -----

  [[nodiscard]] bool Contains(std::string_view word) const {
    return Count(word) > 0;
  }

  [[nodiscard]] bool ContainsWithPrefix(std::string_view prefix) const {
    return CountWithPrefix(prefix) > 0;
  }

  [[nodiscard]] bool ContainsPrefixOf(std::string_view word) const {
    return CountPrefixesOf(word) > 0;
  }

  // ----- Miscellaneous -----

  // Length of the longest common prefix with any stored string. O(|word|).
  [[nodiscard]] int LcpWith(std::string_view word) const {
    int node_index = 0;
    const int len = static_cast<int>(word.size());
    for (int i = 0; i < len; ++i) {
      const int idx = word[i] - kBase;
      if (!IsValidIndex(idx)) {
        return i;
      }
      const int child_index = nodes_[node_index].children[idx];
      if (child_index == kNull) {
        return i;
      }
      node_index = child_index;
    }
    return len;
  }

 private:
  static constexpr int kNull = -1;

  struct Node {
    std::array<int, kNumChar> children;
    CountType prefix_count;
    CountType end_count;

    Node() { Reset(); }

    void Reset() {
      children.fill(kNull);
      prefix_count = 0;
      end_count = 0;
    }
  };

  [[nodiscard]] static constexpr bool IsValidIndex(int idx) {
    return 0 <= idx && idx < kNumChar;
  }

  int NewNode() {
    if (!free_list_.empty()) {
      const int idx = free_list_.back();
      free_list_.pop_back();
      nodes_[idx].Reset();
      return idx;
    }
    nodes_.emplace_back();
    return static_cast<int>(nodes_.size() - 1);
  }

  void ClearSubtree(int node_index) {
    std::vector<int> stack;
    stack.push_back(node_index);
    while (!stack.empty()) {
      const int idx = stack.back();
      stack.pop_back();
      Node& node = nodes_[idx];
      for (int& child : node.children) {
        if (child != kNull) {
          stack.push_back(child);
          child = kNull;
        }
      }
      node.prefix_count = 0;
      node.end_count = 0;
      if (idx != 0) {
        free_list_.push_back(idx);
      }
    }
  }

  void SubtractAlongPath(const std::vector<int>& path, CountType dec) {
    if (dec <= 0) {
      return;
    }
    for (const int idx : path) {
      Node& node = nodes_[idx];
      if (node.prefix_count > dec) {
        node.prefix_count -= dec;
      } else {
        node.prefix_count = 0;
      }
    }
  }

  int FindNode(std::string_view word) const {
    return FindNode(word, nullptr);
  }

  int FindNode(std::string_view word, std::vector<int>* path) const {
    int node_index = 0;
    if (path != nullptr) {
      path->clear();
      path->reserve(word.size() + 1);
      path->push_back(node_index);
    }
    for (const char ch : word) {
      const int idx = ch - kBase;
      if (!IsValidIndex(idx)) {
        return kNull;
      }
      const int child_index = nodes_[node_index].children[idx];
      if (child_index == kNull) {
        return kNull;
      }
      node_index = child_index;
      if (path != nullptr) {
        path->push_back(node_index);
      }
    }
    return node_index;
  }

  std::vector<Node> nodes_;
  std::vector<int> free_list_;
};

}  // namespace hotaosa

#endif  // HOTAOSA_DS_TRIE_H_
