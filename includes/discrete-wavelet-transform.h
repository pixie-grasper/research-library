// Copyright 2015 pixie-grasper
/// \file discrete-wavelet-transform.h
/// \brief Implementation of the DWT
/// \author pixie.grasper

#ifndef INCLUDES_DISCRETE_WAVELET_TRANSFORM_H_
#define INCLUDES_DISCRETE_WAVELET_TRANSFORM_H_

#include <vector>
#include <utility>

/// \namespace ResearchLibrary
/// \namespace ResearchLibrary::DiscreteWaveletTransform
namespace ResearchLibrary {
namespace DiscreteWaveletTransform {

/// \fn Haar(const std::vector<T>& data)
/// \brief Haar's Discrete Wavelet Transform Function
/// \param[in] data sequence
/// \return \c std::pair of approximation part and detail part
///         as \c std::vector<T>
template <typename T>
auto Haar(const std::vector<T>& data) {
  auto N = data.size() / 2;
  std::vector<T> approxim(N), detail(N);
  for (size_t i = 0; i < N; i++) {
    approxim[i] = data[2 * i] + data[2 * i + 1];
    detail[i]   = data[2 * i] - data[2 * i + 1];
  }
  return std::make_pair(approxim, detail);
}

/// \fn IHaar(const std::vector<T>& approxim, const std::vector<T>& detail)
/// \brief Haar's Discrete Wavelet Transform Function
/// \param[in] approxim approximation coefficients
/// \param[in] detail detail coefficients
/// \return data sequence as \c std::vector<T>
template <typename T>
auto IHaar(const std::vector<T>& approxim, const std::vector<T>& detail) {
  auto N = approxim.size();
  std::vector<T> ret(2 * N);
  for (size_t i = 0; i < N; i++) {
    ret[2 * i] = (approxim[i] + detail[i]) / 2;
    ret[2 * i + 1] = (approxim[i] - detail[i]) / 2;
  }
  return ret;
}

/// \fn IHaar(const std::pair<std::vector<T>, std::vector<T>>& pair)
/// \brief Haar's Discrete Wavelet Transform Function
/// \param[in] pair \c std::pair of approximation and detail coefficients
/// \return data sequence as \c std::vector<T>
template <typename T>
auto IHaar(const std::pair<std::vector<T>, std::vector<T>>& pair) {
  return IHaar(pair.first, pair.second);
}

}  // namespace DiscreteWaveletTransform
}  // namespace ResearchLibrary

#endif  // INCLUDES_DISCRETE_WAVELET_TRANSFORM_H_
