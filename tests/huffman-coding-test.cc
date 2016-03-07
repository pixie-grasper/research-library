// Copyright 2015 pixie.grasper

#include <stdio.h>
#include <stdlib.h>

#include <vector>

#include "../includes/huffman-coding.h"

int main() {
  std::vector<int> buffer(10000);
  unsigned int seed = 10;
  for (size_t i = 0; i < buffer.size(); i++) {
    buffer[i] = rand_r(&seed) % 100;
  }

  auto&& nhc = ResearchLibrary::HuffmanCoding::NumericEncode(buffer);
  auto&& ihc = ResearchLibrary::HuffmanCoding::Decode(nhc);
  for (size_t i = 0; i < buffer.size(); i++) {
    if (ihc[i] != buffer[i]) {
      return 1;
    }
  }

  return 0;
}
