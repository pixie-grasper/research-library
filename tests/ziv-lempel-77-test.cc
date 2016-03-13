// Copyright 2015 pixie.grasper

#include <stdio.h>
#include <stdlib.h>

#include <vector>

#include "../includes/ziv-lempel-77.h"

int main() {
  std::vector<int> buffer(10000);
  unsigned int seed = 10;
  for (std::size_t i = 0; i < buffer.size(); i++) {
    buffer[i] = rand_r(&seed) % 100;
  }

  auto&& zl77 = ResearchLibrary::ZivLempel77::Encode(buffer, 3, 128);
  auto&& izl77 = ResearchLibrary::ZivLempel77::Decode(zl77);
  for (std::size_t i = 0; i < buffer.size(); i++) {
    if (izl77[i] != buffer[i]) {
      return 1;
    }
  }

  return 0;
}
