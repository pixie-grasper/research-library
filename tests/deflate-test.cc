// Copyright 2015 pixie.grasper

#include <cstdlib>
#include <cstdint>
#include <vector>

#include "../includes/deflate.h"

int main() {
  std::vector<std::uint8_t> buffer(10000);
  unsigned int seed = 10;
  for (std::size_t i = 0; i < buffer.size(); i++) {
    if (rand_r(&seed) % 10 == 0) {
      buffer[i] = 0;
    } else {
      buffer[i] = 1;
    }
  }

  auto&& deflate = ResearchLibrary::Deflate::Encode(buffer);
  auto&& inflate = ResearchLibrary::Deflate::Decode(deflate);
  for (std::size_t i = 0; i < inflate.size(); i++) {
    if (buffer[i] != inflate[i]) {
      return 1;
    }
  }
  return 0;
}
