// Copyright 2015 pixie.grasper

#include <stdio.h>
#include <stdlib.h>

#include <vector>

#include "../includes/prediction-by-partial-matching.h"

int main() {
  std::vector<int> buffer(10000);
  unsigned int seed = 10;
  for (size_t i = 0; i < buffer.size(); i++) {
    buffer[i] = rand_r(&seed) % 100;
  }

  auto&& ppm = ResearchLibrary::PredictionByPartialMatching
                              ::Encode<int, 2, MethodA>(buffer);
  auto&& ippm = ResearchLibrary::PredictionByPartialMatching
                               ::Decode<int, 2, MethodA>(ppm);
  for (size_t i = 0; i < buffer.size(); i++) {
    if (ippm[i] != buffer[i]) {
      return 1;
    }
  }

  return 0;
}
