// Copyright 2015 pixie.grasper

#include <stdio.h>
#include <stdlib.h>

#include <vector>

#include "../includes/ziv-lempel-77.h"

int main() {
#if 0
  std::vector<int> buffer{'x', 'y', 'z', 'x', 'y', 'a', 'x', 'y', 'z', '$'};
#elif 0
  std::vector<int> buffer
    {'m', 'i', 's', 's', 'i', 's', 's', 'i', 'p', 'p', 'i', '$'};
#else
  std::vector<int> buffer(200);
  unsigned int seed = 10;
  for (std::size_t i = 0; i < buffer.size(); i++) {
    buffer[i] = rand_r(&seed) % 2;
  }
#endif

  auto&& zl77 = ResearchLibrary::ZivLempel77::Encode(buffer, 1);
  for (std::size_t i = 0; i < zl77.size(); i++) {
    if (zl77[i].matched) {
      fprintf(stderr, "(%zu %zu)\n", zl77[i].first, zl77[i].length);
    } else {
      fprintf(stderr, "%d\n", zl77[i].character);
    }
  }
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
