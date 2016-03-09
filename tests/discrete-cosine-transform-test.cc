// Copyright 2015 pixie.grasper

#include <cstdlib>
#include <vector>

#include "../includes/discrete-cosine-transform.h"

int main() {
  std::vector<double> buffer(1024);
  unsigned int seed = 10;
  for (size_t i = 0; i < buffer.size(); i++) {
    buffer[i] = rand_r(&seed) % 100;
  }
  auto&& dct = ResearchLibrary::DiscreteCosineTransform::DCT(buffer);
  auto&& idct = ResearchLibrary::DiscreteCosineTransform::IDCT(dct);
  for (size_t i = 0; i < buffer.size(); i++) {
    if (buffer[i] != 0.0 &&
        std::abs((idct[i] - buffer[i]) / buffer[i]) > 1e-10) {
      return 1;
    } else if (buffer[i] == 0.0 && std::abs(idct[i]) > 1e-10) {
      return 1;
    }
  }
  return 0;
}
