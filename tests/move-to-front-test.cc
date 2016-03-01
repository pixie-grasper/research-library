// Copyright 2015 pixie-grasper

#include <stdio.h>
#include <stdlib.h>

#include <vector>

#include "../move-to-front.h"

int main() {
  std::vector<int> buffer(10000);
  unsigned int seed = 10;
  for (size_t i = 0; i < buffer.size(); i++) {
    buffer[i] = rand_r(&seed) % 100;
  }
  {
    auto&& mtf = ResearchLibrary::MoveToFront::MTF(buffer);
    auto&& imtf = ResearchLibrary::MoveToFront::IMTF(mtf);
    for (size_t i = 0; i < imtf.size(); i++) {
      if (imtf[i] != buffer[i]) {
        return 1;
      }
    }
  }
  {
    auto dictionary_size = size_t(30);
    auto&& mtf = ResearchLibrary::MoveToFront::MTF(buffer, dictionary_size);
    auto&& imtf = ResearchLibrary::MoveToFront::IMTF(mtf, dictionary_size);
    for (size_t i = 0; i < imtf.size(); i++) {
      if (imtf[i] != buffer[i]) {
        return 1;
      }
    }
  }
  {
    auto&& mtf = ResearchLibrary::MoveToFront::NumericMTF(buffer);
    auto&& imtf = ResearchLibrary::MoveToFront::NumericIMTF<int>(mtf);
    for (size_t i = 0; i < imtf.size(); i++) {
      if (imtf[i] != buffer[i]) {
        return 1;
      }
    }
  }
  return 0;
}
