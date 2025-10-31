#include "hotaosa/ds/trie.h"

#include <cstdint>
#include <string_view>

#include <gtest/gtest.h>

namespace hotaosa {
namespace {

using SmallTrie = Trie<26, 'a'>;  // NOLINT

TEST(TrieTest, InsertAndQueryCounts) {
  SmallTrie trie;
  trie.Insert("abc");
  trie.Insert("abc");
  trie.Insert("abd");

  EXPECT_EQ(trie.TotalCount(), 3);
  EXPECT_EQ(trie.Count("abc"), 2);
  EXPECT_EQ(trie.Count("abd"), 1);
  EXPECT_EQ(trie.Count("abe"), 0);

  EXPECT_EQ(trie.CountWithPrefix(""), 3);
  EXPECT_EQ(trie.CountWithPrefix("ab"), 3);
  EXPECT_EQ(trie.CountWithPrefix("abc"), 2);
  EXPECT_EQ(trie.CountWithPrefix("abd"), 1);
  EXPECT_EQ(trie.CountWithPrefix("abcd"), 0);
  EXPECT_EQ(trie.CountWithPrefix("abe"), 0);

  EXPECT_EQ(trie.CountPrefixesOf("abc"), 2);
  EXPECT_EQ(trie.CountPrefixesOf("abdz"), 1);
  EXPECT_EQ(trie.CountPrefixesOf("abe"), 0);
  EXPECT_EQ(trie.CountPrefixesOf("abd"), 1);
  EXPECT_EQ(trie.CountPrefixesOf("cab"), 0);

  EXPECT_TRUE(trie.Contains("abc"));
  EXPECT_TRUE(trie.Contains("abd"));
  EXPECT_FALSE(trie.Contains("abe"));

  EXPECT_TRUE(trie.ContainsWithPrefix("ab"));
  EXPECT_TRUE(trie.ContainsWithPrefix(""));
  EXPECT_TRUE(trie.ContainsPrefixOf("abc"));
  EXPECT_TRUE(trie.ContainsPrefixOf("abdz"));
  EXPECT_FALSE(trie.ContainsPrefixOf("cab"));
  EXPECT_EQ(trie.TotalCount(), 3);
}

TEST(TrieTest, RemoveClampsCounts) {
  SmallTrie trie;
  trie.Insert("abc");
  trie.Insert("abd");
  trie.Insert("abd");

  trie.Remove("abc", 5);  // NOLINT
  EXPECT_EQ(trie.Count("abc"), 0);
  EXPECT_EQ(trie.CountWithPrefix("ab"), 2);
  EXPECT_EQ(trie.CountPrefixesOf("abdz"), 2);

  trie.Remove("abd");
  EXPECT_EQ(trie.Count("abd"), 1);
  EXPECT_EQ(trie.CountWithPrefix("ab"), 1);
  EXPECT_EQ(trie.CountPrefixesOf("abd"), 1);

  trie.Remove("abd");
  EXPECT_EQ(trie.Count("abd"), 0);
  EXPECT_EQ(trie.CountWithPrefix("ab"), 0);
  EXPECT_EQ(trie.CountPrefixesOf("abd"), 0);
  EXPECT_FALSE(trie.ContainsWithPrefix("ab"));
  EXPECT_FALSE(trie.ContainsPrefixOf("abd"));
  EXPECT_EQ(trie.TotalCount(), 0);
}

TEST(TrieTest, RemoveWithPrefixErasesSubtree) {
  SmallTrie trie;
  trie.Insert("abc", 2);
  trie.Insert("abd");
  trie.Insert("b");

  trie.RemoveWithPrefix("ab");

  EXPECT_EQ(trie.Count("abc"), 0);
  EXPECT_EQ(trie.Count("abd"), 0);
  EXPECT_EQ(trie.Count("b"), 1);

  EXPECT_EQ(trie.CountWithPrefix("ab"), 0);
  EXPECT_EQ(trie.CountWithPrefix(""), 1);
  EXPECT_EQ(trie.CountPrefixesOf("abc"), 0);
  EXPECT_TRUE(trie.Contains("b"));
  EXPECT_FALSE(trie.ContainsWithPrefix("ab"));
  EXPECT_FALSE(trie.ContainsPrefixOf("abz"));
  EXPECT_EQ(trie.TotalCount(), 1);

  // Removing a missing prefix changes nothing.
  trie.RemoveWithPrefix("ab");
  EXPECT_EQ(trie.Count("b"), 1);
  EXPECT_EQ(trie.TotalCount(), 1);

  trie.Insert("", 3);
  EXPECT_EQ(trie.CountWithPrefix(""), 4);
  EXPECT_EQ(trie.TotalCount(), 4);

  // Removing the empty prefix clears the entire structure.
  trie.RemoveWithPrefix("");
  EXPECT_EQ(trie.CountWithPrefix(""), 0);
  EXPECT_FALSE(trie.Contains("b"));
  EXPECT_EQ(trie.TotalCount(), 0);
}

TEST(TrieTest, RemovePrefixesOfErasesPrefixChain) {
  SmallTrie trie;
  trie.Insert("");
  trie.Insert("a");
  trie.Insert("ab", 2);
  trie.Insert("abc");
  trie.Insert("abd");
  trie.Insert("b");

  trie.RemovePrefixesOf("abz");
  EXPECT_EQ(trie.Count(""), 0);
  EXPECT_EQ(trie.Count("a"), 0);
  EXPECT_EQ(trie.Count("ab"), 0);
  EXPECT_EQ(trie.Count("abc"), 1);
  EXPECT_EQ(trie.Count("abd"), 1);
  EXPECT_EQ(trie.Count("b"), 1);
  EXPECT_EQ(trie.CountWithPrefix("ab"), 2);
  EXPECT_EQ(trie.TotalCount(), 3);

  trie.RemovePrefixesOf("abc");
  EXPECT_EQ(trie.Count("abc"), 0);
  EXPECT_EQ(trie.CountWithPrefix("ab"), 1);
  EXPECT_EQ(trie.TotalCount(), 2);

  trie.RemovePrefixesOf("");
  EXPECT_EQ(trie.Count(""), 0);
  EXPECT_EQ(trie.Count("abd"), 1);
  EXPECT_EQ(trie.CountWithPrefix(""), 2);
  EXPECT_EQ(trie.TotalCount(), 2);
}

using LargeTrie = Trie<10, '0', std::int64_t>;  // NOLINT

TEST(TrieTest, SupportsLargeCountsAndCustomType) {
  LargeTrie trie;
  const std::int64_t big = 1'000'000'000'000LL;
  trie.Insert("123", big);
  EXPECT_EQ(trie.Count("123"), big);
  EXPECT_EQ(trie.CountWithPrefix("1"), big);
  EXPECT_EQ(trie.CountPrefixesOf("123456"), big);

  trie.Remove("123", big - 1);
  EXPECT_EQ(trie.Count("123"), 1);
  EXPECT_EQ(trie.CountWithPrefix("12"), 1);
  EXPECT_EQ(trie.CountPrefixesOf("123"), 1);

  trie.Remove("123");
  EXPECT_EQ(trie.Count("123"), 0);
  EXPECT_EQ(trie.CountWithPrefix("1"), 0);
}

}  // namespace
}  // namespace hotaosa
