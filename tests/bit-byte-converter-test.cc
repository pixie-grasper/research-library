// Copyright 2015 pixie.grasper

#include "../includes/bit-byte-converter.h"

int main() {
  size_t L = 13;
  auto N = size_t(1) << L;
  ResearchLibrary::BitsToBytes<8> conv{};
  for (size_t i = 0; i < N; i++) {
    conv.put(i, L);
  }
  auto buffer = conv.seek_to_byte_boundary();
  ResearchLibrary::BytesToBits<8> iconv(buffer);
  for (size_t i = 0; i < N; i++) {
    auto v = iconv.get(L);
    if (v != i) {
      return 1;
    }
  }

  return 0;
}
