// Copyright 2015 pixie-grasper

#ifndef INCLUDES_FAST_FOURIER_TRANSFORM_H_
#define INCLUDES_FAST_FOURIER_TRANSFORM_H_

int gets();
#include <vector>
#include <map>
#include <utility>
#include <algorithm>
#include <complex>

#ifdef RESEARCHLIB_OFFLINE_TEST
#include "./size-type.h"
#else
#include <size-type.h>
#endif

namespace ResearchLibrary {
namespace FastFourierTransform {

template <size_t N>
auto width_map() {
  std::map<size_type_t<N>, size_t> w;
  for (size_t i = 0; i < N * 8; i++) {
    w[size_type_t<N>(1) << i] = i;
  }
  return w;
}

template <size_t N>
size_type_t<N> upside_down(size_type_t<N> x, size_t width);

template <>
size_type_t<1> upside_down<1>(size_type_t<1> x, size_t width) {
  x = size_type_t<1>((x & 0x55) << 1) | size_type_t<1>((x & 0xaa) >> 1);
  x = size_type_t<1>((x & 0x33) << 2) | size_type_t<1>((x & 0xcc) >> 2);
  x = size_type_t<1>((x & 0x0f) << 4) | size_type_t<1>((x & 0xf0) >> 4);
  return x >> (8 - width);
}

template <>
size_type_t<2> upside_down<2>(size_type_t<2> x, size_t width) {
  x = size_type_t<2>((x & 0x5555) << 1) | size_type_t<2>((x & 0xaaaa) >> 1);
  x = size_type_t<2>((x & 0x3333) << 2) | size_type_t<2>((x & 0xcccc) >> 2);
  x = size_type_t<2>((x & 0x0f0f) << 4) | size_type_t<2>((x & 0xf0f0) >> 4);
  x = size_type_t<2>((x & 0x00ff) << 8) | size_type_t<2>((x & 0xff00) >> 8);
  return x >> (16 - width);
}

template <>
size_type_t<4> upside_down<4>(size_type_t<4> x, size_t width) {
  x = ((x & 0x55555555) << 1)  | ((x & 0xaaaaaaaa) >> 1);
  x = ((x & 0x33333333) << 2)  | ((x & 0xcccccccc) >> 2);
  x = ((x & 0x0f0f0f0f) << 4)  | ((x & 0xf0f0f0f0) >> 4);
  x = ((x & 0x00ff00ff) << 8)  | ((x & 0xff00ff00) >> 8);
  x = ((x & 0x0000ffff) << 16) | ((x & 0xffff0000) >> 16);
  return x >> (32 - width);
}

template <>
size_type_t<8> upside_down<8>(size_type_t<8> x, size_t width) {
  x = ((x & 0x5555555555555555) << 1)  | ((x & 0xaaaaaaaaaaaaaaaa) >> 1);
  x = ((x & 0x3333333333333333) << 2)  | ((x & 0xcccccccccccccccc) >> 2);
  x = ((x & 0x0f0f0f0f0f0f0f0f) << 4)  | ((x & 0xf0f0f0f0f0f0f0f0) >> 4);
  x = ((x & 0x00ff00ff00ff00ff) << 8)  | ((x & 0xff00ff00ff00ff00) >> 8);
  x = ((x & 0x0000ffff0000ffff) << 16) | ((x & 0xffff0000ffff0000) >> 16);
  x = ((x & 0x00000000ffffffff) << 32) | ((x & 0xffffffff00000000) >> 32);
  return x >> (64 - width);
}

template <typename T>
auto FFT(const std::vector<T>& re, const std::vector<T>& im) {
  auto N = re.size();
  auto width = width_map<sizeof(size_t)>()[N];
  std::vector<T> t1re(N), t1im(N), t2re(N), t2im(N), c(N), s(N);
  for (size_t i = 0; i < N; i++) {
    c[i] = cos(2 * M_PI * i / N);
    s[i] = -sin(2 * M_PI * i / N);
    auto j = upside_down<sizeof(size_t)>(i, width);
    t1re[i] = re[j];
    t1im[i] = im[j];
  }
  for (size_t n = 1; n < N; n <<= 1) {
    std::swap(t1re, t2re);
    std::swap(t1im, t2im);
    auto k = N / n / 2;
    for (size_t i = 0; i < N; i += 2 * n) {
      for (size_t j = 0; j < n; j++) {
        auto cij = c[(i + j) * k % N], sij = s[(i + j) * k % N];
        auto cijn = c[(i + j + n) * k % N], sijn = s[(i + j + n) * k % N];
        t1re[i + j]     = t2re[i + j] + t2re[i + j + n] * cij
                                      - t2im[i + j + n] * sij;
        t1im[i + j]     = t2im[i + j] + t2im[i + j + n] * cij
                                      + t2re[i + j + n] * sij;
        t1re[i + j + n] = t2re[i + j] + t2re[i + j + n] * cijn
                                      - t2im[i + j + n] * sijn;
        t1im[i + j + n] = t2im[i + j] + t2im[i + j + n] * cijn
                                      + t2re[i + j + n] * sijn;
      }
    }
  }
  return make_pair(t1re, t1im);
}

template <typename T>
auto FFT(const std::vector<std::complex<T>>& data) {
  auto N = data.size();
  auto width = width_map<sizeof(size_t)>()[N];
  std::vector<std::complex<T>> t1(N), t2(N), e(N);
  for (size_t i = 0; i < N; i++) {
    e[i] = std::complex<T>(cos(2 * M_PI * i / N), -sin(2 * M_PI * i / N));
    auto j = upside_down<sizeof(size_t)>(i, width);
    t1[i] = data[j];
  }
  for (size_t n = 1; n < N; n <<= 1) {
    std::swap(t1, t2);
    auto k = N / n / 2;
    for (size_t i = 0; i < N; i += 2 * n) {
      for (size_t j = 0; j < n; j++) {
        t1[i + j]     = t2[i + j] + t2[i + j + n] * e[(i + j) * k % N];
        t1[i + j + n] = t2[i + j] + t2[i + j + n] * e[(i + j + n) * k % N];
      }
    }
  }
  return t1;
}

template <typename T>
auto IFFT(const std::vector<T>& re, const std::vector<T>& im) {
  auto N = re.size();
  auto width = width_map<sizeof(size_t)>()[N];
  std::vector<T> t1re(N), t1im(N), t2re(N), t2im(N), c(N), s(N);
  for (size_t i = 0; i < N; i++) {
    c[i] = cos(2 * M_PI * i / N);
    s[i] = -sin(2 * M_PI * i / N);
    auto j = upside_down<sizeof(size_t)>(i, width);
    t1re[i] = re[j];
    t1im[i] = -im[j];
  }
  for (size_t n = 1; n < N; n <<= 1) {
    std::swap(t1re, t2re);
    std::swap(t1im, t2im);
    auto k = N / n / 2;
    for (size_t i = 0; i < N; i += 2 * n) {
      for (size_t j = 0; j < n; j++) {
        auto cij = c[(i + j) * k % N], sij = s[(i + j) * k % N];
        auto cijn = c[(i + j + n) * k % N], sijn = s[(i + j + n) * k % N];
        t1re[i + j]     = t2re[i + j] + t2re[i + j + n] * cij
                                      - t2im[i + j + n] * sij;
        t1im[i + j]     = t2im[i + j] + t2im[i + j + n] * cij
                                      + t2re[i + j + n] * sij;
        t1re[i + j + n] = t2re[i + j] + t2re[i + j + n] * cijn
                                      - t2im[i + j + n] * sijn;
        t1im[i + j + n] = t2im[i + j] + t2im[i + j + n] * cijn
                                      + t2re[i + j + n] * sijn;
      }
    }
  }
  for (size_t i = 0; i < N; i++) {
    t1re[i] /= static_cast<double>(N);
    t1im[i] /= -static_cast<double>(N);
  }
  return make_pair(t1re, t1im);
}

template <typename T>
auto IFFT(const std::pair<std::vector<T>, std::vector<T>>& pair) {
  return IFFT(pair.first, pair.second);
}

template <typename T>
auto IFFT(const std::vector<std::complex<T>>& data) {
  auto N = data.size();
  auto width = width_map<sizeof(size_t)>()[N];
  std::vector<std::complex<T>> t1(N), t2(N), e(N);
  for (size_t i = 0; i < N; i++) {
    e[i] = std::complex<T>(cos(2 * M_PI * i / N), -sin(2 * M_PI * i / N));
    auto j = upside_down<sizeof(size_t)>(i, width);
    t1[i] = std::conj(data[j]);
  }
  for (size_t n = 1; n < N; n <<= 1) {
    std::swap(t1, t2);
    auto k = N / n / 2;
    for (size_t i = 0; i < N; i += 2 * n) {
      for (size_t j = 0; j < n; j++) {
        t1[i + j]     = t2[i + j] + t2[i + j + n] * e[(i + j) * k % N];
        t1[i + j + n] = t2[i + j] + t2[i + j + n] * e[(i + j + n) * k % N];
      }
    }
  }
  for (size_t i = 0; i < N; i++) {
    t1[i] = std::conj(t1[i]) / static_cast<double>(N);
  }
  return t1;
}

}  // namespace FastFourierTransform
}  // namespace ResearchLibrary

#endif  // INCLUDES_FAST_FOURIER_TRANSFORM_H_
