// Copyright 2015 pixie.grasper

#include <cstdlib>

#include "../includes/lempel-ziv-storer-szymanski.h"

int main() {
  std::vector<int> buffer(10000);
  unsigned int seed = 10;
  for (std::size_t i = 0; i < buffer.size(); i++) {
    buffer[i] = rand_r(&seed) % 2;
  }

  auto&& lzss = ResearchLibrary::LempelZivStorerSzymanski
                               ::Encode(buffer, 3, 285, 32768);
  auto&& ilzss = ResearchLibrary::LempelZivStorerSzymanski
                                ::Decode(lzss);
  for (std::size_t i = 0; i < buffer.size(); i++) {
    if (ilzss[i] != buffer[i]) {
      return 1;
    }
  }
  return 0;
}
