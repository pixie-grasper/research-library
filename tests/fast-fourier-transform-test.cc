// Copyright 2015 pixie.grasper


int gets();
#include <cstdlib>
#include <vector>
#include <complex>

#include "../includes/fast-fourier-transform.h"

int main() {
  {
    std::vector<std::complex<double>> v(1024);
    unsigned int seed = 10;
    for (size_t i = 0; i < v.size(); i++) {
      v[i] = std::complex<double>(rand_r(&seed), rand_r(&seed));
    }
    auto&& fft = ResearchLibrary::FastFourierTransform::FFT(v);
    auto&& ifft = ResearchLibrary::FastFourierTransform::IFFT(fft);
    for (size_t i = 0; i < ifft.size(); i++) {
      if (std::abs((ifft[i] - v[i]) / v[i]) > 1e-10) {
        return 1;
      }
    }
  }
  {
    std::vector<double> re(1024), im(1024);
    unsigned int seed = 10;
    for (size_t i = 0; i < re.size(); i++) {
      re[i] = rand_r(&seed);
      im[i] = rand_r(&seed);
    }
    auto&& fft = ResearchLibrary::FastFourierTransform::FFT(re, im);
    auto&& ifft = ResearchLibrary::FastFourierTransform::IFFT(fft);
    for (size_t i = 0; i < re.size(); i++) {
      if (std::abs((ifft.first[i] - re[i]) / re[i]) > 1e-10) {
        return 1;
      } else if (std::abs((ifft.second[i] - im[i]) / im[i]) > 1e-10) {
        return 1;
      }
    }
  }
  return 0;
}
