// Copyright 2015 pixie-grasper
/// \file discrete-wavelet-transform.h
/// \brief Implementation of the DWT
/// \author pixie.grasper

#ifndef INCLUDES_DISCRETE_WAVELET_TRANSFORM_H_
#define INCLUDES_DISCRETE_WAVELET_TRANSFORM_H_

#include <cstddef>
#include <vector>
#include <utility>
#include <cstdlib>

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
  for (std::size_t i = 0; i < N; i++) {
    approxim[i] = data[2 * i] + data[2 * i + 1];
    detail[i]   = data[2 * i] - data[2 * i + 1];
  }
  return std::make_pair(std::move(approxim), std::move(detail));
}

/// \fn IHaar(const std::vector<T>& approxim, const std::vector<T>& detail)
/// \brief Inverse Haar's Discrete Wavelet Transform Function
/// \param[in] approxim approximation coefficients
/// \param[in] detail detail coefficients
/// \return data sequence as \c std::vector<T>
template <typename T>
auto IHaar(const std::vector<T>& approxim, const std::vector<T>& detail) {
  auto N = approxim.size();
  std::vector<T> ret(2 * N);
  for (std::size_t i = 0; i < N; i++) {
    ret[2 * i] = (approxim[i] + detail[i]) / 2;
    ret[2 * i + 1] = (approxim[i] - detail[i]) / 2;
  }
  return ret;
}

/// \fn IHaar(const std::pair<std::vector<T>, std::vector<T>>& pair)
/// \brief Inverse Haar's Discrete Wavelet Transform Function
/// \param[in] pair \c std::pair of approximation and detail coefficients
/// \return data sequence as \c std::vector<T>
template <typename T>
auto IHaar(const std::pair<std::vector<T>, std::vector<T>>& pair) {
  return IHaar(pair.first, pair.second);
}

/// \fn CDF53(const std::vector<T>& data)
/// \brief Cohen-Daubechies-Feauveau 5/3 Wavelet Transform Function
/// \param[in] data sequence
/// \return \c std::pair of approximation part and detail part
///         as \c std::vector<T>
template <typename T>
auto CDF53(const std::vector<T>& data) {
  constexpr auto a = -0.5;
  constexpr auto b = 0.25;
  auto N = data.size() / 2;
  std::vector<T> even(N), odd(N);
  for (std::size_t i = 0; i < N; i++) {
    even[i] = data[2 * i];
    odd[i] = data[2 * i + 1];
  }
  for (std::size_t i = 0; i < N; i++) {
    odd[i] += a * (even[i] + even[(i + 1) % N]);
  }
  for (std::size_t i = 0; i < N; i++) {
    even[i] += b * (odd[i] + odd[(i - 1 + N) % N]);
  }
  return std::make_pair(std::move(even), std::move(odd));
}

/// \fn ICDF53(const std::vector<T>& approxim, const std::vector<T>& detail)
/// \brief Inverse Cohen-Daubechies-Feauveau 5/3 Wavelet Transform Function
/// \param[in] approxim approximation coefficients
/// \param[in] detail detail coefficients
/// \return data sequence as \c std::vector<T>
template <typename T>
auto ICDF53(const std::vector<T>& approxim, const std::vector<T>& detail) {
  constexpr auto a = -0.5;
  constexpr auto b = 0.25;
  auto N = approxim.size();
  std::vector<T> even(N), odd(N);
  for (std::size_t i = 0; i < N; i++) {
    even[i] = approxim[i];
    odd[i] = detail[i];
  }
  for (std::size_t i = 0; i < N; i++) {
    even[i] -= b * (odd[i] + odd[(i - 1 + N) % N]);
  }
  for (std::size_t i = 0; i < N; i++) {
    odd[i] -= a * (even[i] + even[(i + 1) % N]);
  }
  std::vector<T> ret(2 * N);
  for (std::size_t i = 0; i < N; i++) {
    ret[2 * i] = even[i];
    ret[2 * i + 1] = odd[i];
  }
  return ret;
}

/// \fn ICDF53(const std::pair<std::vector<T>, std::vector<T>>& pair)
/// \brief Inverse Cohen-Daubechies-Feauveau 5/3 Wavelet Transform Function
/// \param[in] pair \c std::pair of approximation and detail coefficients
/// \return data sequence as \c std::vector<T>
template <typename T>
auto ICDF53(const std::pair<std::vector<T>, std::vector<T>>& pair) {
  return ICDF53(pair.first, pair.second);
}

/// \fn CDF97(const std::vector<T>& data)
/// \brief Cohen-Daubechies-Feauveau 9/7 Wavelet Transform Function
/// \param[in] data sequence
/// \return \c std::pair of approximation part and detail part
///         as \c std::vector<T>
template <typename T>
auto CDF97(const std::vector<T>& data) {
  constexpr auto a = -1.5861343420693648;
  constexpr auto b = -0.0529801185718856;
  constexpr auto c = 0.8829110755411875;
  constexpr auto d = 0.4435068520511142;
  auto N = data.size() / 2;
  std::vector<T> even(N), odd(N);
  for (std::size_t i = 0; i < N; i++) {
    even[i] = data[2 * i];
    odd[i] = data[2 * i + 1];
  }
  for (std::size_t i = 0; i < N; i++) {
    odd[i] += a * (even[i] + even[(i + 1) % N]);
  }
  for (std::size_t i = 0; i < N; i++) {
    even[i] += b * (odd[i] + odd[(i - 1 + N) % N]);
  }
  for (std::size_t i = 0; i < N; i++) {
    odd[i] += c * (even[i] + even[(i + 1) % N]);
  }
  for (std::size_t i = 0; i < N; i++) {
    even[i] += d * (odd[i] + odd[(i - 1 + N) % N]);
  }
  return std::make_pair(std::move(even), std::move(odd));
}

/// \fn ICDF97(const std::vector<T>& approxim, const std::vector<T>& detail)
/// \brief Inverse Cohen-Daubechies-Feauveau 9/7 Wavelet Transform Function
/// \param[in] approxim approximation coefficients
/// \param[in] detail detail coefficients
/// \return data sequence as \c std::vector<T>
template <typename T>
auto ICDF97(const std::vector<T>& approxim, const std::vector<T>& detail) {
  constexpr auto a = -1.5861343420693648;
  constexpr auto b = -0.0529801185718856;
  constexpr auto c = 0.8829110755411875;
  constexpr auto d = 0.4435068520511142;
  auto N = approxim.size();
  std::vector<T> even(N), odd(N);
  for (std::size_t i = 0; i < N; i++) {
    even[i] = approxim[i];
    odd[i] = detail[i];
  }
  for (std::size_t i = 0; i < N; i++) {
    even[i] -= d * (odd[i] + odd[(i - 1 + N) % N]);
  }
  for (std::size_t i = 0; i < N; i++) {
    odd[i] -= c * (even[i] + even[(i + 1) % N]);
  }
  for (std::size_t i = 0; i < N; i++) {
    even[i] -= b * (odd[i] + odd[(i - 1 + N) % N]);
  }
  for (std::size_t i = 0; i < N; i++) {
    odd[i] -= a * (even[i] + even[(i + 1) % N]);
  }
  std::vector<T> ret(2 * N);
  for (std::size_t i = 0; i < N; i++) {
    ret[2 * i] = even[i];
    ret[2 * i + 1] = odd[i];
  }
  return ret;
}

/// \fn ICDF97(const std::pair<std::vector<T>, std::vector<T>>& pair)
/// \brief Inverse Cohen-Daubechies-Feauveau 9/7 Wavelet Transform Function
/// \param[in] pair \c std::pair of approximation and detail coefficients
/// \return data sequence as \c std::vector<T>
template <typename T>
auto ICDF97(const std::pair<std::vector<T>, std::vector<T>>& pair) {
  return ICDF97(pair.first, pair.second);
}

}  // namespace DiscreteWaveletTransform
}  // namespace ResearchLibrary

#endif  // INCLUDES_DISCRETE_WAVELET_TRANSFORM_H_
