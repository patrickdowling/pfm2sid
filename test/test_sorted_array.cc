#include <set>
#include <vector>

#include "fmt/core.h"
#include "gtest/gtest.h"
#include "misc/sorted_array.h"

namespace pfm2sid::test {

template <typename C>
void Dump(const C &container)
{
  fmt::print("{} [ ", container.size());
  for (auto &v : container) { fmt::print("{} ", v); }
  fmt::println("]");
}

TEST(SortedArrayTest, Basic)
{
  util::StaticSortedArray<int, 8> arr;
  EXPECT_EQ(0, arr.size());
  EXPECT_EQ(8, arr.capacity());

  std::vector<int> items = {1, 7, 5, 3};

  for (auto i : items) arr.insert(i);

  EXPECT_EQ(items.size(), arr.size());
  Dump(arr);

  items.push_back(0);
  items.push_back(9);
  items.push_back(4);

  // This should not re-add existing items, but insert the new ones
  for (auto i : items) arr.insert(i);

  EXPECT_EQ(items.size(), arr.size());
  Dump(arr);

  std::set<int> sorted_items{items.begin(), items.end()};

  auto ai = arr.begin();
  for (auto i : sorted_items) {
    EXPECT_EQ(i, *ai);
    ++ai;

    auto pos = std::find(arr.begin(), arr.end(), i);
    EXPECT_NE(arr.end(), pos);
  }

  arr.erase(0);
  arr.erase(3);
  arr.erase(9);
  EXPECT_EQ(4, arr.size());
  Dump(arr);

  for (int i = 0; i < 10; ++i) arr.erase(i);

  EXPECT_EQ(0, arr.size());

  for (int i = 0; i < 32; ++i) arr.insert(i);
  EXPECT_EQ(8, arr.size());
  Dump(arr);
}

}  // namespace pfm2sid::test
