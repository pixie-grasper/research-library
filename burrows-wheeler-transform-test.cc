// Copyright 2015 pixie.grasper

#include <stdio.h>
#include <stdlib.h>

#include <vector>

#include "./burrows-wheeler-transform.h"

int main() {
  std::vector<int> buffer(10000);
  unsigned int seed = 10;
  for (size_t i = 0; i < buffer.size(); i++) {
    buffer[i] = rand_r(&seed) % 100;
  }
  auto&& bwt = ResearchLibrary::BurrowsWheelerTransform::BWT(buffer);
  auto&& ibwt = ResearchLibrary::BurrowsWheelerTransform::IBWT(bwt);
  for (size_t i = 0; i < buffer.size(); i++) {
    if (ibwt[i] != buffer[i]) {
      return 1;
    }
  }
  return 0;
}
