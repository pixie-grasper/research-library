// Copyright 2015 pixie.grasper

#include <stdio.h>
#include <stdlib.h>

#include <vector>

#include "../includes/prediction-by-partial-matching.h"

int main() {
  std::vector<int> buffer(1000);
  unsigned int seed = 10;
  for (size_t i = 0; i < buffer.size(); i++) {
    buffer[i] = rand_r(&seed) % 100;
  }

  auto&& ppma = ResearchLibrary::PredictionByPartialMatching
                               ::Encode<MethodA, 2>(buffer);
  auto&& ippma = ResearchLibrary::PredictionByPartialMatching
                                ::Decode<MethodA, 2>(ppma);
  for (size_t i = 0; i < buffer.size(); i++) {
    if (ippma[i] != buffer[i]) {
      return 1;
    }
  }

  auto&& ppmb = ResearchLibrary::PredictionByPartialMatching
                               ::Encode<MethodB, 2>(buffer);
  auto&& ippmb = ResearchLibrary::PredictionByPartialMatching
                                ::Decode<MethodB, 2>(ppmb);
  for (size_t i = 0; i < buffer.size(); i++) {
    if (ippmb[i] != buffer[i]) {
      return 1;
    }
  }

  auto&& ppmc = ResearchLibrary::PredictionByPartialMatching
                               ::Encode<MethodC, 2>(buffer);
  auto&& ippmc = ResearchLibrary::PredictionByPartialMatching
                                ::Decode<MethodC, 2>(ppmc);
  for (size_t i = 0; i < buffer.size(); i++) {
    if (ippmc[i] != buffer[i]) {
      return 1;
    }
  }

  auto&& ppmd = ResearchLibrary::PredictionByPartialMatching
                               ::Encode<MethodD, 2>(buffer);
  auto&& ippmd = ResearchLibrary::PredictionByPartialMatching
                                ::Decode<MethodD, 2>(ppmd);
  for (size_t i = 0; i < buffer.size(); i++) {
    if (ippmd[i] != buffer[i]) {
      return 1;
    }
  }

  return 0;
}
