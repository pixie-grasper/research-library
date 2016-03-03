// Copyright 2015 pixie.grasper

#include <stdio.h>
#include <stdlib.h>

#include <vector>

#include "../includes/range-coder.h"

int main() {
  std::vector<int> buffer(10000);
  unsigned int seed = 10;
  for (size_t i = 0; i < buffer.size(); i++) {
    buffer[i] = rand_r(&seed) % 100;
  }

  auto&& src = ResearchLibrary::RangeCoder::StaticEncode(buffer);
  auto&& isrc = ResearchLibrary::RangeCoder::StaticDecode(src);
  for (size_t i = 0; i < buffer.size(); i++) {
    if (isrc[i] != buffer[i]) {
      return 1;
    }
  }

  auto&& arca = ResearchLibrary::RangeCoder::AdaptiveEncodeA(buffer);
  auto&& iarca = ResearchLibrary::RangeCoder::AdaptiveDecodeA(arca);
  for (size_t i = 0; i < buffer.size(); i++) {
    if (iarca[i] != buffer[i]) {
      return 1;
    }
  }

  auto&& arcb = ResearchLibrary::RangeCoder::AdaptiveEncodeB(buffer);
  auto&& iarcb = ResearchLibrary::RangeCoder::AdaptiveDecodeB(arcb);
  for (size_t i = 0; i < buffer.size(); i++) {
    if (iarcb[i] != buffer[i]) {
      return 1;
    }
  }

  auto&& arcc = ResearchLibrary::RangeCoder::AdaptiveEncodeC(buffer);
  auto&& iarcc = ResearchLibrary::RangeCoder::AdaptiveDecodeC(arcc);
  for (size_t i = 0; i < buffer.size(); i++) {
    if (iarcc[i] != buffer[i]) {
      return 1;
    }
  }

  auto&& arcd = ResearchLibrary::RangeCoder::AdaptiveEncodeD(buffer);
  auto&& iarcd = ResearchLibrary::RangeCoder::AdaptiveDecodeD(arcd);
  for (size_t i = 0; i < buffer.size(); i++) {
    if (iarcd[i] != buffer[i]) {
      return 1;
    }
  }

  return 0;
}
