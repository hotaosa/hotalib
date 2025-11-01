#include "hotaosa/ds/binary_trie.h"

#include <cstdint>

#include "gtest/gtest.h"

namespace hotaosa {
namespace {

TEST(BinaryTrieTest, InsertEraseAndCount) {
  BinaryTrie<std::uint32_t> trie;
  EXPECT_EQ(trie.TotalCount(), 0);

  trie.Insert(5);
  trie.Insert(0);
  trie.Insert(5);

  EXPECT_EQ(trie.TotalCount(), 3);
  EXPECT_TRUE(trie.Contains(5));
  EXPECT_EQ(trie.Count(5), 2);
  EXPECT_EQ(trie.Count(0), 1);
  EXPECT_FALSE(trie.Contains(7));

  trie.Erase(5);
  EXPECT_EQ(trie.Count(5), 1);
  EXPECT_EQ(trie.TotalCount(), 2);

  trie.Erase(5, 10);
  EXPECT_EQ(trie.Count(5), 0);
  EXPECT_EQ(trie.TotalCount(), 1);

  trie.Erase(123);  // missing key, no-op
  EXPECT_EQ(trie.TotalCount(), 1);
}

TEST(BinaryTrieTest, KthHandlesDuplicates) {
  BinaryTrie<std::uint8_t, 6, int> trie;
  trie.Insert(1);
  trie.Insert(4, 2);
  trie.Insert(7);

  ASSERT_TRUE(trie.Kth(0).has_value());
  EXPECT_EQ(trie.Kth(0).value(), 1);
  ASSERT_TRUE(trie.Kth(1).has_value());
  EXPECT_EQ(trie.Kth(1).value(), 4);
  ASSERT_TRUE(trie.Kth(2).has_value());
  EXPECT_EQ(trie.Kth(2).value(), 4);
  ASSERT_TRUE(trie.Kth(3).has_value());
  EXPECT_EQ(trie.Kth(3).value(), 7);
  EXPECT_FALSE(trie.Kth(4).has_value());
}

TEST(BinaryTrieTest, CountLessAndGreater) {
  BinaryTrie<std::uint16_t, 8> trie;
  trie.Insert(1);
  trie.Insert(3, 2);
  trie.Insert(7);

  EXPECT_EQ(trie.CountLess(0), 0);
  EXPECT_EQ(trie.CountLess(1), 0);
  EXPECT_EQ(trie.CountLess(3), 1);
  EXPECT_EQ(trie.CountLess(4), 3);

  EXPECT_EQ(trie.CountGreater(7), 0);
  EXPECT_EQ(trie.CountGreater(6), 1);
  EXPECT_EQ(trie.CountGreater(2), 3);
}

TEST(BinaryTrieTest, LowerBoundAndPrev) {
  BinaryTrie<std::uint16_t, 10> trie;
  trie.Insert(12);
  trie.Insert(20);
  trie.Insert(31);

  ASSERT_TRUE(trie.LowerBound(0).has_value());
  EXPECT_EQ(trie.LowerBound(0).value(), 12);

  ASSERT_TRUE(trie.LowerBound(19).has_value());
  EXPECT_EQ(trie.LowerBound(19).value(), 20);

  EXPECT_FALSE(trie.LowerBound(40).has_value());

  ASSERT_TRUE(trie.Prev(25).has_value());
  EXPECT_EQ(trie.Prev(25).value(), 20);

  ASSERT_TRUE(trie.Prev(12).has_value());
  EXPECT_EQ(trie.Prev(12).value(), 12);

  EXPECT_FALSE(trie.Prev(5).has_value());
}

TEST(BinaryTrieTest, MaxMinXorQueries) {
  BinaryTrie<std::uint32_t> trie;
  trie.Insert(1);
  trie.Insert(2);
  trie.Insert(4);

  ASSERT_TRUE(trie.MaxXor(6).has_value());
  EXPECT_EQ(trie.MaxXor(6).value(), 6 ^ 1);  // best element is 1 -> xor = 7

  ASSERT_TRUE(trie.MinXor(6).has_value());
  EXPECT_EQ(trie.MinXor(6).value(), 6 ^ 4);  // best element is 4 -> xor = 2
}

TEST(BinaryTrieTest, XorAllReinterpretsKeys) {
  BinaryTrie<std::uint16_t, 8> trie;
  trie.Insert(1);
  trie.Insert(6);

  trie.XorAll(3);  // {1,6} -> {1^3=2, 6^3=5}
  EXPECT_TRUE(trie.Contains(2));
  EXPECT_TRUE(trie.Contains(5));
  EXPECT_FALSE(trie.Contains(1));

  EXPECT_EQ(trie.CountLess(5), 1);
  EXPECT_EQ(trie.CountGreater(2), 1);

  ASSERT_TRUE(trie.LowerBound(0).has_value());
  EXPECT_EQ(trie.LowerBound(0).value(), 2);

  ASSERT_TRUE(trie.Prev(5).has_value());
  EXPECT_EQ(trie.Prev(5).value(), 5);

  ASSERT_TRUE(trie.MaxXor(1).has_value());
  EXPECT_EQ(trie.MaxXor(1).value(), 1 ^ 5);  // element 5 maximises XOR
}

}  // namespace
}  // namespace hotaosa
