// Copyright 2015 pixie.grasper

#include <stdio.h>
#include <stdlib.h>

int gets();
#include <vector>
#include <algorithm>

#include "../includes/discrete-wavelet-transform.h"

int main() {
  std::vector<double> buffer(1024);
  unsigned int seed = 10;
  for (size_t i = 0; i < buffer.size(); i++) {
    buffer[i] = rand_r(&seed) % 100;
  }
  auto&& haar = ResearchLibrary::DiscreteWaveletTransform::Haar(buffer);
  auto&& ihaar = ResearchLibrary::DiscreteWaveletTransform::IHaar(haar);
  for (size_t i = 0; i < buffer.size(); i++) {
    if (buffer[i] != 0.0 &&
        std::abs((ihaar[i] - buffer[i]) / buffer[i]) > 1e-10) {
      return 1;
    } else if (buffer[i] == 0.0 && std::abs(ihaar[i]) > 1e-10) {
      return 1;
    }
  }
  return 0;
}
