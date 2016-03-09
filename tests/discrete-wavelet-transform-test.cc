// Copyright 2015 pixie.grasper

#include <cmath>

int gets();
#include <vector>
#include <algorithm>

#include "../includes/discrete-wavelet-transform.h"

bool has_big_error(const std::vector<double>& orig,
                   const std::vector<double>& rep) {
  for (size_t i = 0; i < orig.size(); i++) {
    if (orig[i] != 0.0 &&
        std::abs((rep[i] - orig[i]) / orig[i]) > 1e-10) {
      return true;
    } else if (orig[i] == 0.0 && std::abs(rep[i]) > 1e-10) {
      return true;
    }
  }
  return false;
}

int main() {
  std::vector<double> buffer(1024);
  unsigned int seed = 10;
  for (size_t i = 0; i < buffer.size(); i++) {
    buffer[i] = rand_r(&seed) % 100;
  }

  auto&& haar = ResearchLibrary::DiscreteWaveletTransform::Haar(buffer);
  auto&& ihaar = ResearchLibrary::DiscreteWaveletTransform::IHaar(haar);
  if (has_big_error(buffer, ihaar)) {
    return 1;
  }

  auto&& cdf97 = ResearchLibrary::DiscreteWaveletTransform::CDF97(buffer);
  auto&& icdf97 = ResearchLibrary::DiscreteWaveletTransform::ICDF97(cdf97);
  if (has_big_error(buffer, icdf97)) {
    return 1;
  }

  return 0;
}
