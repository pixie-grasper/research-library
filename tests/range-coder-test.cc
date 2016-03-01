// Copyright 2015 pixie.grasper

#include <stdio.h>
#include <stdlib.h>

#include <vector>

#include "../range-coder.h"

int main() {
  std::vector<int> buffer(10000);
  unsigned int seed = 10;
  for (size_t i = 0; i < buffer.size(); i++) {
    buffer[i] = rand_r(&seed) % 100;
  }
  auto&& src = ResearchLibrary::RangeCoder::StaticEncode(buffer);
  auto&& isrc = ResearchLibrary::RangeCoder::StaticDecode(src, buffer.size());
  for (size_t i = 0; i < buffer.size(); i++) {
    if (isrc[i] != buffer[i]) {
      return 1;
    }
  }
  return 0;
}
