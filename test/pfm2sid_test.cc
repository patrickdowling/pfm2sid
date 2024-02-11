#include "fmt/core.h"
#include "gtest/gtest.h"

namespace pfm2sid::test {

void Hexdump(const uint8_t *data, size_t len)
{
  char ascii[17] = {0};

  size_t b = 0;
  while (b < len) {
    if (0 == b % 16) {
      if (b) {
        fmt::println(" {}", ascii);
        memset(ascii, 0, sizeof(ascii));
      }
      fmt::print("{:04} |", b);
    }
    auto byte = data[b];
    fmt::print(" {:02x}", byte);
    ascii[b % 16] = isprint(byte) ? byte : '.';
    ++b;
  }

  while (b++ % 16) fmt::print("   ");
  if (ascii[0]) fmt::println(" {}", ascii);
}

}  // namespace pfm2sid::test

int main(int argc, char **argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
