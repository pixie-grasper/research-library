// Copyright 2015 pixie-grasper
/// \file discrete-cosine-transform.h
/// \brief Implementation of the DCT
/// \author pixie.grasper

#ifndef INCLUDES_DISCRETE_COSINE_TRANSFORM_H_
#define INCLUDES_DISCRETE_COSINE_TRANSFORM_H_

#include <vector>

#ifdef RESEARCHLIB_OFFLINE_TEST
#include "./fast-fourier-transform.h"
#else
#include <fast-fourier-transform.h>
#endif

/// \namespace ResearchLibrary
/// \namespace ResearchLibrary::DiscreteCosineTransform
namespace ResearchLibrary {
namespace DiscreteCosineTransform {

/// \fn DCT(const std::vector<T>& data)
/// \brief Discrete-Cosine Transform Function
/// \param[in] data sequence
/// \return specturm data as std::vector<T>
template <typename T>
auto DCT(const std::vector<T>& data) {
  auto N = data.size() * 4;
  std::vector<T> re(N, 0), im(N, 0);
  for (size_t i = 0; i < data.size(); i++) {
    re[2 * i + 1] = re[N - 2 * i - 1] = data[i];
  }
  auto&& ret = FastFourierTransform::FFT(re, im).first;
  ret.resize(data.size());
  return ret;
}

/// \fn IDCT(const std::vector<T>& data)
/// \brief Inverse Discrete-Cosine Transform Function
/// \param[in] data spectrum sequence
/// \return data as std::vector<T>
template <typename T>
auto IDCT(const std::vector<T>& data) {
  auto N = data.size() * 4;
  std::vector<T> re(N, 0), im(N, 0);
  re[0] = data[0];
  re[N / 2] = -data[0];
  for (size_t i = 1; i < data.size(); i++) {
    re[i] = re[N - i] = data[i];
    re[N / 2 + i] = re[N / 2 - i] = -data[i];
  }
  auto&& ret = FastFourierTransform::IFFT(re, im).first;
  for (size_t i = 0; i < data.size(); i++) {
    ret[i] = ret[2 * i + 1];
  }
  ret.resize(data.size());
  return ret;
}

}  // namespace DiscreteCosineTransform
}  // namespace ResearchLibrary

#endif  // INCLUDES_DISCRETE_COSINE_TRANSFORM_H_
