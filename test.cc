// Copyright 2015 pixie.grasper

#include <stdio.h>
#include <stdlib.h>

#include <vector>
#include <utility>

#include "./burrows-wheeler-transform.h"
#include "./range-coder.h"

int main() {
  std::vector<int> buffer(100000);
  unsigned int seed = 10;
  for (size_t i = 0; i < buffer.size(); i++) {
    buffer[i] = rand_r(&seed) % 2;
  }
  auto&& bwt = ResearchLibrary::BurrowsWheelerTransform::BWT(buffer);
  auto&& ibwt = ResearchLibrary::BurrowsWheelerTransform::IBWT(bwt);
  for (size_t i = 0; i < buffer.size(); i++) {
    if (ibwt[i] != buffer[i]) {
      fprintf(stderr, "%d %d %zu\n", ibwt[i], buffer[i], i);
    }
  }
  return 0;
}
