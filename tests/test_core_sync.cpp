#include "../libraries/core/sync.hpp"

#include <cassert>
#include <vector>

bool TestSerializeLong() {
  long long_value = 0x0001000012345678;
  std::vector<u_int8_t> expected_result = {0x78, 0x56, 0x34, 0x12,
                                           0x00, 0x00, 0x01, 0x00};

  std::vector<u_int8_t> result = SerializeLong(long_value);

  bool are_equal = (result == expected_result);
  assert(are_equal);
}

bool TestGenerateBuf() {
  long mtype = 0x02;
  std::vector<u_int8_t> data = {0x01, 0x02, 0xFF};
  std::vector<u_int8_t> expected_buf = {0x02, 0x00, 0x00, 0x00, 0x00,
                                        0x00, 0x00, 0x00, 0x01, 0x02, 0xFF};

  std::vector<u_int8_t> buf = GenerateBuf(mtype, data);

  bool are_equal = (buf == expected_buf);
  assert(are_equal);
}

int main() {
  assert(TestSerializeLong());
  assert(TestGenerateBuf());
}