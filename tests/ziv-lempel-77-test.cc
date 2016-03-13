// Copyright 2015 pixie.grasper

#include <stdio.h>
#include <stdlib.h>

#include <vector>

#include "../includes/ziv-lempel-77.h"

int main() {
  std::vector<int> buffer(1000);
  unsigned int seed = 10;
  for (std::size_t i = 0; i < buffer.size(); i++) {
    buffer[i] = rand_r(&seed) % 2;
  }

  auto&& zl77 = ResearchLibrary::ZivLempel77::Encode(buffer, 3, 128);
#if 1
  for (std::size_t i = 0; i < zl77.size(); i++) {
    fprintf(stderr, "(%zu %zu %d)\n",
        zl77[i].start, zl77[i].length, zl77[i].character);
  }
#endif
#if 0
  auto&& ilz77 = ResearchLibrary::ZivLempel77::Decode(zl77);
  for (std::size_t i = 0; i < buffer.size(); i++) {
    if (izl77[i] != buffer[i]) {
      return 1;
    }
  }
#endif

  return 0;
}
