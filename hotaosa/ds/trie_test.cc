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
