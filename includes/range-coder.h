// Copyright 2015 pixie.grasper
/// \file range-coder.h
/// \brief Implementation of the RangeCoder
/// \author pixie.grasper

#ifndef INCLUDES_RANGE_CODER_H_
#define INCLUDES_RANGE_CODER_H_

/// \privatesection
int gets();
/// \publicsection
#include <cstdint>
#include <vector>
#include <map>
#include <utility>

#ifdef RESEARCHLIB_OFFLINE_TEST
#include "./size-type.h"
#else
#include <size-type.h>
#endif

/// \namespace ResearchLibrary
/// \namespace ResearchLibrary::RangeCoder
namespace ResearchLibrary {
namespace RangeCoder {

/// \privatesection
template <std::size_t N>
auto mulhi(size_type_t<N> u, size_type_t<N> v) {
  auto u0 = u & size_type<N / 2>::max, u1 = u >> N * 8 / 2;
  auto v0 = v & size_type<N / 2>::max, v1 = v >> N * 8 / 2;
  auto w0 = u0 * v0;
  auto t = u1 * v0 + (w0 >> N * 8 / 2);
  auto w1 = t & size_type<N / 2>::max, w2 = t >> N * 8 / 2;
  w1 = u0 * v1 + w1;
  return u1 * v1 + w2 + (w1 >> N * 8 / 2);
}

template <std::size_t N>
auto idiv(size_type_t<N> x, size_type_t<N> z) {
  size_type_t<N> y = 0;
  for (std::size_t i = 0; i < N * 8; i++) {
    auto t = false;
    if (x & ~(size_type<N>::max >> 1)) {
      t = true;
    }
    x = (x << 1) | (y >> 63);
    y <<= 1;
    if (t || x >= z) {
      x -= z;
      y++;
    }
  }
  return y;
}

template <std::size_t N>
struct EncoderContinuation {
  std::vector<std::uint8_t> buffer;
  size_type_t<N> low, range;
};

template <std::size_t N>
auto encode_init() {
  EncoderContinuation<N> cont{};
  cont.range--;
  return cont;
}

template <std::size_t N>
auto encode_process(EncoderContinuation<N>&& cont,
                    size_type_t<N> low,
                    size_type_t<N> range) {
  auto new_low = cont.low + mulhi<N>(cont.range, low);
  auto new_range = mulhi<N>(cont.range, range);
  if (new_low < cont.low) {
    cont.buffer.back()++;
    for (auto i = cont.buffer.size() - 1; cont.buffer[i] == 0; i--) {
      cont.buffer[i - 1]++;
    }
  }
  cont.low = new_low;
  cont.range = new_range;
  while (cont.range < ((size_type_t<N>(1) << (N - 1) * 8) - 1)) {
    cont.buffer.push_back(cont.low >> (N - 1) * 8);
    cont.low <<= 8;
    cont.range <<= 8;
  }
  return std::move(cont);
}

template <std::size_t N>
auto encode_process(EncoderContinuation<N>&& cont,
                    const size_type_t<N>& low,
                    const size_type_t<N>& range,
                    const size_type_t<N>& sum) {
  auto fixed_low = idiv<N>(low, sum);
  auto fixed_range = idiv<N>(range, sum);
  return encode_process(std::move(cont), fixed_low, fixed_range);
}

template <std::size_t N>
auto encode_finish(EncoderContinuation<N>&& cont) {
  auto new_low = cont.low + cont.range / 2;
  if (new_low < cont.low) {
    cont.buffer.back()++;
    for (auto i = cont.buffer.size() - 1; cont.buffer[i] == 0; i--) {
      cont.buffer[i - 1]++;
    }
  }
  cont.low = new_low;
  cont.buffer.push_back((cont.low >> (N - 1) * 8) & 0xff);
  cont.buffer.push_back((cont.low >> (N - 2) * 8) & 0xff);
  return std::move(cont.buffer);
}

template <std::size_t N, typename T>
struct DecoderContinuation {
  size_type_t<N> low, range, data, index;
  T buffer;
};

template <std::size_t N, typename T>
auto decode_init(const std::vector<std::uint8_t>& data) {
  DecoderContinuation<N, T> cont{};
  cont.range = size_type_t<N>(UINT64_MAX);
  for (std::size_t i = 0; i < N; i++) {
    if (i < data.size()) {
      cont.data = (cont.data << 8) + data[i];
    } else {
      cont.data <<= 8;
    }
  }
  cont.index = 8;
  return cont;
}

// adaptive frequency
template <std::size_t N, typename T>
auto decode_split(const DecoderContinuation<N, T>& cont,
                  std::size_t border,
                  std::size_t sum) {
  if (border == sum) {
    return false;
  }
  auto ch_in = idiv<N>(cont.data - cont.low, cont.range);
  auto fixed_border = idiv<N>(border, sum);
  if (fixed_border <= ch_in) {
    return true;  // right
  } else {
    return false;
  }
}

template <std::size_t N, typename T>
auto decode_partial_fetch(const DecoderContinuation<N, T>& cont,
                          const std::vector<unsigned_integer_t>& freq,
                          std::size_t total_freq,
                          std::size_t sum) {
  auto ch_in_exact = idiv<N>(cont.data - cont.low, cont.range);
  auto ch_in = mulhi<N>(ch_in_exact, sum);
  if (idiv<N>(total_freq, sum) <= ch_in_exact) {
    return std::make_pair(T(0), false);
  }
  std::size_t ch = 0, sum_freq = freq[0];
  while (sum_freq <= ch_in) {
    ch++;
    sum_freq += freq[ch];
  }
  return std::make_pair(T(ch), true);
}

template <std::size_t N, typename T>
auto decode_fetch(const DecoderContinuation<N, T>& cont,
                  const std::vector<unsigned_integer_t>& freq,
                  std::size_t sum) {
  auto ch_in = mulhi<N>(idiv<N>(cont.data - cont.low, cont.range), sum);
  std::size_t ch = 0, sum_freq = freq[0];
  while (sum_freq <= ch_in) {
    ch++;
    sum_freq += freq[ch];
  }
  return T(ch);
}

template <std::size_t N, typename T>
auto decode_fetch(const DecoderContinuation<N, T>& cont, std::size_t sum) {
  return mulhi<N>(idiv<N>(cont.data - cont.low, cont.range), sum);
}

template <std::size_t N, typename T>
auto decode_process(DecoderContinuation<N, T>&& cont,
                    const std::vector<std::uint8_t>& data,
                    const unsigned_integer_t low,
                    const unsigned_integer_t range,
                    std::size_t sum) {
  auto fixed_low = idiv<N>(low, sum);
  auto fixed_range = idiv<N>(range, sum);
  auto new_low = cont.low + mulhi<N>(cont.range, fixed_low);
  auto new_range = mulhi<N>(cont.range, fixed_range);
  cont.low = new_low;
  cont.range = new_range;
  while (cont.range < ((size_type_t<N>(1) << (N - 1) * 8) - 1)) {
    cont.data <<= 8;
    cont.low <<= 8;
    cont.range <<= 8;
    if (cont.index < data.size()) {
      cont.data += data[cont.index];
      cont.index++;
    }
  }
  return std::move(cont);
}

// fixed frequency
template <std::size_t N, typename T>
auto decode_process(DecoderContinuation<N, T>&& cont,
                    const std::vector<std::uint8_t>& data,
                    const std::vector<std::pair<T, unsigned_integer_t>>& freq,
                    const std::vector<unsigned_integer_t>& sum_freq) {
  auto sum = sum_freq.back() + freq.back().second;
  auto ch_in = mulhi<N>(idiv<N>(cont.data - cont.low, cont.range), sum);
  std::size_t left = 0, right = freq.size() - 1;
  while (left < right) {
    auto mid = (left + right) / 2;
    if (sum_freq[mid + 1] <= ch_in) {
      left = mid + 1;
    } else {
      right = mid;
    }
  }
  auto ch = left;
  auto low = idiv<N>(sum_freq[ch], sum);
  auto range = idiv<N>(freq[ch].second, sum);
  auto new_low = cont.low + mulhi<N>(cont.range, low);
  auto new_range = mulhi<N>(cont.range, range);
  cont.low = new_low;
  cont.range = new_range;
  cont.buffer = freq[ch].first;
  while (cont.range < ((size_type_t<N>(1) << (N - 1) * 8) - 1)) {
    cont.data <<= 8;
    cont.low <<= 8;
    cont.range <<= 8;
    if (cont.index < data.size()) {
      cont.data += data[cont.index];
      cont.index++;
    }
  }
  return std::move(cont);
}

/// \publicsection
/// \fn StaticEncode(const std::vector<T>& data,
///                  const std::vector<std::pair<T, unsigned_integer_t>>& freq)
/// \brief RangeCoder Static Encode Function
/// \param[in] data sequence
/// \param[in] freq list of frequency of the characters
/// \return encoded sequence as \c std::vector<std::uint8_t>
template <typename T>
auto StaticEncode(const std::vector<T>& data,
                  const std::vector<std::pair<T, unsigned_integer_t>>& freq) {
  std::map<T, unsigned_integer_t> sum_freq, freq_map;
  unsigned_integer_t sum = 0;
  for (auto&& it = freq.begin(); it != freq.end(); ++it) {
    sum_freq[it->first] = sum;
    freq_map[it->first] = it->second;
    sum += it->second;
  }
  auto&& cont = encode_init<unsigned_integer_size>();
  for (std::size_t i = 0; i < data.size(); i++) {
    cont = encode_process(std::move(cont),
                          sum_freq[data[i]],
                          freq_map[data[i]],
                          sum);
  }
  return encode_finish(std::move(cont));
}

/// \fn StaticEncode(const std::vector<T>& data)
/// \brief RangeCoder Static Encode Function
/// \param[in] data sequence
/// \return std::pair of encoded sequence as \c std::vector<std::uint8_t> and
///         list of frequency of the characters as
///         std::vector<std::pair<T, unsigned_integer_t>>
template <typename T>
auto StaticEncode(const std::vector<T>& data) {
  std::map<T, unsigned_integer_t> freq_map;
  std::vector<std::pair<T, unsigned_integer_t>> freq;
  for (auto&& it = data.begin(); it != data.end(); ++it) {
    freq_map[*it]++;
  }
  for (auto&& it = freq_map.begin(); it != freq_map.end(); ++it) {
    freq.push_back(*it);
  }
  auto&& encoded = StaticEncode(data, freq);
  return std::make_pair(std::move(encoded),
                        std::make_pair(std::move(freq),
                                       data.size()));
}

/// \fn StaticDecode(const std::vector<std::uint8_t>& data,
///                  const std::vector<std::pair<T, unsigned_integer_t>>& freq,
///                  std::size_t original_size)
/// \brief RangeCoder Static Decode Function
/// \param[in] data sequence
/// \param[in] freq list of frequency of the characters on the source
/// \param[in] original_size length of the original sequence
/// \return original sequence as std::vector<T>
template <typename T>
auto StaticDecode(const std::vector<std::uint8_t>& data,
                  const std::vector<std::pair<T, unsigned_integer_t>>& freq,
                  std::size_t original_size) {
  std::vector<unsigned_integer_t> sum_freq;
  unsigned_integer_t sum = 0;
  for (auto&& it = freq.begin(); it != freq.end(); ++it) {
    sum_freq.push_back(sum);
    sum += it->second;
  }
  std::vector<T> ret;
  auto&& cont = decode_init<unsigned_integer_size, T>(data);
  for (std::size_t i = 0; i < original_size; i++) {
    cont = decode_process(std::move(cont), data, freq, sum_freq);
    ret.push_back(cont.buffer);
  }
  return ret;
}

/// \fn StaticDecode(
///       const std::pair<std::vector<std::uint8_t>,
///                       std::pair<std::vector<std::pair<T,
///                                                       unsigned_integer_t>>,
///                                 std::size_t>>& tuple)
/// \brief RangeCoder Static Decode Function
/// \param[in] tuple std::pair of data sequence as std::vector<std::uint8_t> and
///            std::pair of list of frequency of the characters on the source as
///            std::vector<std::pair<T, unsigned_integer_t>> and data length as
///            std::size_t
/// \return original sequence as std::vector<T>
template <typename T>
auto StaticDecode(const std::pair<
                          std::vector<std::uint8_t>,
                          std::pair<
                            std::vector<std::pair<T, unsigned_integer_t>>,
                            std::size_t>>& tuple) {
  return StaticDecode(tuple.first, tuple.second.first, tuple.second.second);
}

/// \fn AdaptiveEncodeA(const std::vector<T>& data, const T& max)
/// \brief RangeCoder Adaptive Encode Function.
///        To solve the zero-frequency-problem, use the Method A.
/// \param[in] data sequence
/// \param[in] max maximum value of the data
/// \return \c std::pair of sequence as \c std::vector<std::uint8_t> and
///         length of the data sequence as \c std::size_t
template <typename T>
auto AdaptiveEncodeA(const std::vector<T>& data, const T& max) {
  auto A = std::size_t(max + 1);
  std::vector<unsigned_integer_t> sum_freq(A, 0), freq(A, 0);
  std::vector<unsigned_integer_t> sum_nfreq(A, 0), nfreq(A, 1);
  unsigned_integer_t sum = 0;
  for (std::size_t i = 0; i < sum_nfreq.size(); i++) {
    sum_nfreq[i] = sum;
    sum += nfreq[i];
  }
  std::size_t incorrect_sum_freq_min = 0, u = 0;
  auto&& cont = encode_init<unsigned_integer_size>();
  for (std::size_t i = 0; i < data.size(); i++) {
    auto n = i + 1;
    auto d = std::size_t(data[i]);
    if (freq[d] == 0) {
      cont = encode_process(std::move(cont), i, 1, n);
      cont = encode_process(std::move(cont), sum_nfreq[d], 1, A - u);
      nfreq[d] = 0;
      for (auto j = d + 1; j < sum_nfreq.size(); j++) {
        sum_nfreq[j] = sum_nfreq[j - 1] + nfreq[j - 1];
      }
      u++;
    } else {
      if (incorrect_sum_freq_min <= d) {
        if (incorrect_sum_freq_min == 0) {
          incorrect_sum_freq_min = 1;
        }
        for (auto j = incorrect_sum_freq_min; j <= d; j++) {
          sum_freq[j] = sum_freq[j - 1] + freq[j - 1];
        }
        incorrect_sum_freq_min = d + 1;
      }
      cont = encode_process(std::move(cont), sum_freq[d], freq[d], n);
    }
    freq[d]++;
    if (d < incorrect_sum_freq_min) {
      incorrect_sum_freq_min = d;
    }
  }
  return std::make_pair(encode_finish(std::move(cont)), data.size());
}

/// \fn AdaptiveEncodeA(const std::vector<T>& data)
/// \brief RangeCoder Adaptive Encode Function.
///        To solve the zero-frequency-problem, use the Method A.
/// \param[in] data sequence
/// \return \c std::pair of sequence as \c std::vector<std::uint8_t> and
///         \c std::pair of length of the original sequence as \c std::size_t
///         and maximum value of the original sequence as \c T
template <typename T>
auto AdaptiveEncodeA(const std::vector<T>& data) {
  T max{};
  for (std::size_t i = 0; i < data.size(); i++) {
    if (max < data[i]) {
      max = data[i];
    }
  }
  auto&& pair = AdaptiveEncodeA(data, max);
  return std::make_pair(std::move(pair.first),
                        std::make_pair(std::move(pair.second),
                                       std::move(max)));
}

/// \fn AdaptiveDecodeA(const std::vector<std::uint8_t>& data,
///                    std::size_t original_size,
///                    const T& max)
/// \brief RangeCoder Adaptive Decode Function.
///        To solve the zero-frequency-problem, use the Method A.
/// \param[in] data sequence
/// \param[in] original_size length of the original sequence
/// \param[in] max maximum value of the original sequence
/// \return original sequence as \c std::vector<T>
template <typename T>
auto AdaptiveDecodeA(const std::vector<std::uint8_t>& data,
                     std::size_t original_size,
                     const T& max) {
  auto A = std::size_t(max + 1);
  std::vector<unsigned_integer_t> sum_freq(A, 0), freq(A, 0);
  std::vector<unsigned_integer_t> sum_nfreq(A, 0), nfreq(A, 1);
  unsigned_integer_t sum = 0;
  for (std::size_t i = 0; i < sum_nfreq.size(); i++) {
    sum_nfreq[i] = sum;
    sum += nfreq[i];
  }
  std::size_t incorrect_sum_freq_min = 0, u = 0;
  std::vector<T> ret;
  auto&& cont = decode_init<unsigned_integer_size, T>(data);
  for (std::size_t i = 0; i < original_size; i++) {
    auto n = i + 1;
    auto decode = decode_partial_fetch(cont, freq, i, n);
    auto d = std::size_t(decode.first);
    if (!decode.second) {
      cont = decode_process(std::move(cont), data, i, 1, n);
      auto Td = decode_fetch(cont, nfreq, A - u);
      d = std::size_t(Td);
      cont = decode_process(std::move(cont), data, sum_nfreq[d], 1, A - u);
      nfreq[d] = 0;
      for (auto j = d + 1; j < sum_nfreq.size(); j++) {
        sum_nfreq[j] = sum_nfreq[j - 1] + nfreq[j - 1];
      }
      ret.push_back(Td);
      u++;
    } else {
      if (incorrect_sum_freq_min <= d) {
        if (incorrect_sum_freq_min == 0) {
          incorrect_sum_freq_min = 1;
        }
        for (auto j = incorrect_sum_freq_min; j <= d; j++) {
          sum_freq[j] = sum_freq[j - 1] + freq[j - 1];
        }
        incorrect_sum_freq_min = d + 1;
      }
      cont = decode_process(std::move(cont), data, sum_freq[d], freq[d], n);
      ret.push_back(decode.first);
    }
    freq[d]++;
    if (d < incorrect_sum_freq_min) {
      incorrect_sum_freq_min = d;
    }
  }
  return ret;
}

/// \fn AdaptiveDecodeA(const std::pair<
///                            std::vector<std::uint8_t>,
///                            std::pair<std::size_t, T>>& pair)
/// \brief RangeCoder Adaptive Decode Function.
///        To solve the zero-frequency-problem, use the Method A.
/// \param[in] pair \c std::pair of sequence as std::vector<std::uint8_t> and
///            \c std::pair of length of the original sequence as
///            \c std::size_t and maximum value of the original sequence
/// \return original sequence as \c std::vector<T>
template <typename T>
auto AdaptiveDecodeA(const std::pair<std::vector<std::uint8_t>,
                           std::pair<std::size_t, T>>& tuple) {
  return AdaptiveDecodeA(tuple.first, tuple.second.first, tuple.second.second);
}

/// \fn AdaptiveEncodeB(const std::vector<T>& data, const T& max)
/// \brief RangeCoder Adaptive Encode Function.
///        To solve the zero-frequency-problem, use the Method B.
/// \param[in] data sequence
/// \param[in] max maximum value of the data
/// \return \c std::pair of sequence as \c std::vector<std::uint8_t> and
///         length of the data sequence as \c std::size_t
template <typename T>
auto AdaptiveEncodeB(const std::vector<T>& data, const T& max) {
  auto A = std::size_t(max + 1);
  std::vector<unsigned_integer_t> sum_freq(A, 0), freq(A, 0), freqm1(A, 0);
  std::vector<unsigned_integer_t> sum_nfreq(A, 0), nfreq(A, 1);
  std::vector<unsigned_integer_t> sum_nfreq2(A, 0), nfreq2(A, 0);
  unsigned_integer_t sum = 0;
  for (std::size_t i = 0; i < sum_nfreq.size(); i++) {
    sum_nfreq[i] = sum;
    sum += nfreq[i];
  }
  std::size_t incorrect_sum_freq_min = 0, u = 0, v = 0;
  auto&& cont = encode_init<unsigned_integer_size>();
  for (std::size_t i = 0; i < data.size(); i++) {
    auto d = std::size_t(data[i]);
    if (freq[d] <= 1) {
      if (i != 0) {
        cont = encode_process(std::move(cont), i - u, u, i);
      }
      auto n = A - u + v;
      if (freq[d] == 0) {
        cont = encode_process(std::move(cont), sum_nfreq[d], 1, n);
        nfreq[d] = 0;
        nfreq2[d] = 1;
        for (auto j = d + 1; j < sum_nfreq2.size(); j++) {
          sum_nfreq[j]  = sum_nfreq[j - 1]  + nfreq[j - 1];
          sum_nfreq2[j] = sum_nfreq2[j - 1] + nfreq2[j - 1];
        }
        u++;
        v++;
      } else {
        cont = encode_process(std::move(cont), A - u, v, n);
        cont = encode_process(std::move(cont), sum_nfreq2[d], 1, v);
        nfreq2[d] = 0;
        for (auto j = d + 1; j < sum_nfreq2.size(); j++) {
          sum_nfreq2[j] = sum_nfreq2[j - 1] + nfreq2[j - 1];
        }
        v--;
      }
    } else {
      if (incorrect_sum_freq_min <= d) {
        if (incorrect_sum_freq_min == 0) {
          incorrect_sum_freq_min = 1;
        }
        for (auto j = incorrect_sum_freq_min; j <= d; j++) {
          sum_freq[j] = sum_freq[j - 1] + freqm1[j - 1];
        }
        incorrect_sum_freq_min = d + 1;
      }
      cont = encode_process(std::move(cont), sum_freq[d], freqm1[d], i);
    }
    freqm1[d] = freq[d];
    freq[d]++;
    if (d < incorrect_sum_freq_min) {
      incorrect_sum_freq_min = d;
    }
  }
  return std::make_pair(encode_finish(std::move(cont)), data.size());
}

/// \fn AdaptiveEncodeB(const std::vector<T>& data)
/// \brief RangeCoder Adaptive Encode Function.
///        To solve the zero-frequency-problem, use the Method B.
/// \param[in] data sequence
/// \return \c std::pair of sequence as \c std::vector<std::uint8_t> and
///         \c std::pair of length of the original sequence as \c std::size_t
///         and maximum value of the original sequence as \c T
template <typename T>
auto AdaptiveEncodeB(const std::vector<T>& data) {
  T max{};
  for (std::size_t i = 0; i < data.size(); i++) {
    if (max < data[i]) {
      max = data[i];
    }
  }
  auto&& pair = AdaptiveEncodeB(data, max);
  return std::make_pair(std::move(pair.first),
                        std::make_pair(std::move(pair.second),
                                       std::move(max)));
}

/// \fn AdaptiveDecodeB(const std::vector<std::uint8_t>& data,
///                    std::size_t original_size,
///                    const T& max)
/// \brief RangeCoder Adaptive Decode Function.
///        To solve the zero-frequency-problem, use the Method B.
/// \param[in] data sequence
/// \param[in] original_size length of the original sequence
/// \param[in] max maximum value of the original sequence
/// \return original sequence as \c std::vector<T>
template <typename T>
auto AdaptiveDecodeB(const std::vector<std::uint8_t>& data,
                     std::size_t original_size,
                     const T& max) {
  auto A = std::size_t(max + 1);
  std::vector<unsigned_integer_t> sum_freq(A, 0), freq(A, 0), freqm1(A, 0);
  std::vector<unsigned_integer_t> sum_nfreq(A, 0), nfreq(A, 1);
  std::vector<unsigned_integer_t> sum_nfreq2(A, 0), nfreq2(A, 0);
  unsigned_integer_t sum = 0;
  for (std::size_t i = 0; i < sum_nfreq.size(); i++) {
    sum_nfreq[i] = sum;
    sum += nfreq[i];
  }
  std::size_t incorrect_sum_freq_min = 0, u = 0, v = 0;
  std::vector<T> ret;
  auto&& cont = decode_init<unsigned_integer_size, T>(data);
  for (std::size_t i = 0; i < original_size; i++) {
    auto decode = decode_partial_fetch(cont, freqm1, i - u, i);
    auto d = std::size_t(decode.first);
    if (!decode.second || i == 0) {
      if (i != 0) {
        cont = decode_process(std::move(cont), data, i - u, u, i);
      }
      auto n = A - u + v;
      decode = decode_partial_fetch(cont, nfreq, A - u, n);
      d = std::size_t(decode.first);
      if (decode.second) {
        cont = decode_process(std::move(cont), data, sum_nfreq[d], 1, n);
        nfreq[d] = 0;
        nfreq2[d] = 1;
        for (auto j = d + 1; j < sum_nfreq2.size(); j++) {
          sum_nfreq[j]  = sum_nfreq[j - 1]  + nfreq[j - 1];
          sum_nfreq2[j] = sum_nfreq2[j - 1] + nfreq2[j - 1];
        }
        u++;
        v++;
      } else {
        cont = decode_process(std::move(cont), data, A - u, v, n);
        decode.first = decode_fetch(cont, nfreq2, v);
        d = std::size_t(decode.first);
        cont = decode_process(std::move(cont), data, sum_nfreq2[d], 1, v);
        nfreq2[d] = 0;
        for (auto j = d + 1; j < sum_nfreq2.size(); j++) {
          sum_nfreq2[j] = sum_nfreq2[j - 1] + nfreq2[j - 1];
        }
        v--;
      }
    } else {
      if (incorrect_sum_freq_min <= d) {
        if (incorrect_sum_freq_min == 0) {
          incorrect_sum_freq_min = 1;
        }
        for (auto j = incorrect_sum_freq_min; j <= d; j++) {
          sum_freq[j] = sum_freq[j - 1] + freqm1[j - 1];
        }
        incorrect_sum_freq_min = d + 1;
      }
      cont = decode_process(std::move(cont), data, sum_freq[d], freqm1[d], i);
    }
    ret.push_back(decode.first);
    freqm1[d] = freq[d];
    freq[d]++;
    if (d < incorrect_sum_freq_min) {
      incorrect_sum_freq_min = d;
    }
  }
  return ret;
}

/// \fn AdaptiveDecodeB(const std::pair<
///                            std::vector<std::uint8_t>,
///                            std::pair<std::size_t, T>>& pair)
/// \brief RangeCoder Adaptive Decode Function.
///        To solve the zero-frequency-problem, use the Method B.
/// \param[in] pair \c std::pair of sequence as std::vector<std::uint8_t> and
///            \c std::pair of length of the original sequence as
///            \c std::size_t and maximum value of the original sequence
/// \return original sequence as \c std::vector<T>
template <typename T>
auto AdaptiveDecodeB(const std::pair<std::vector<std::uint8_t>,
                           std::pair<std::size_t, T>>& tuple) {
  return AdaptiveDecodeB(tuple.first, tuple.second.first, tuple.second.second);
}

/// \fn AdaptiveEncodeC(const std::vector<T>& data, const T& max)
/// \brief RangeCoder Adaptive Encode Function.
///        To solve the zero-frequency-problem, use the Method C.
/// \param[in] data sequence
/// \param[in] max maximum value of the data
/// \return \c std::pair of sequence as \c std::vector<std::uint8_t> and
///         length of the data sequence as \c std::size_t
template <typename T>
auto AdaptiveEncodeC(const std::vector<T>& data, const T& max) {
  auto A = std::size_t(max + 1);
  std::vector<unsigned_integer_t> sum_freq(A, 0), freq(A, 0);
  std::vector<unsigned_integer_t> sum_nfreq(A, 0), nfreq(A, 1);
  unsigned_integer_t sum = 0;
  for (std::size_t i = 0; i < sum_nfreq.size(); i++) {
    sum_nfreq[i] = sum;
    sum += nfreq[i];
  }
  std::size_t incorrect_sum_freq_min = 0, u = 0;
  auto&& cont = encode_init<unsigned_integer_size>();
  for (std::size_t i = 0; i < data.size(); i++) {
    auto n = i + u;
    auto d = std::size_t(data[i]);
    if (freq[d] == 0) {
      if (i != 0) {
        cont = encode_process(std::move(cont), i, u, n);
      }
      cont = encode_process(std::move(cont), sum_nfreq[d], 1, A - u);
      nfreq[d] = 0;
      for (auto j = d + 1; j < sum_nfreq.size(); j++) {
        sum_nfreq[j] = sum_nfreq[j - 1] + nfreq[j - 1];
      }
      u++;
    } else {
      if (incorrect_sum_freq_min <= d) {
        if (incorrect_sum_freq_min == 0) {
          incorrect_sum_freq_min = 1;
        }
        for (auto j = incorrect_sum_freq_min; j <= d; j++) {
          sum_freq[j] = sum_freq[j - 1] + freq[j - 1];
        }
        incorrect_sum_freq_min = d + 1;
      }
      cont = encode_process(std::move(cont), sum_freq[d], freq[d], n);
    }
    freq[d]++;
    if (d < incorrect_sum_freq_min) {
      incorrect_sum_freq_min = d;
    }
  }
  return std::make_pair(encode_finish(std::move(cont)), data.size());
}

/// \fn AdaptiveEncodeC(const std::vector<T>& data)
/// \brief RangeCoder Adaptive Encode Function.
///        To solve the zero-frequency-problem, use the Method C.
/// \param[in] data sequence
/// \return \c std::pair of sequence as \c std::vector<std::uint8_t> and
///         \c std::pair of length of the original sequence as \c std::size_t
///         and maximum value of the original sequence as \c T
template <typename T>
auto AdaptiveEncodeC(const std::vector<T>& data) {
  T max{};
  for (std::size_t i = 0; i < data.size(); i++) {
    if (max < data[i]) {
      max = data[i];
    }
  }
  auto&& pair = AdaptiveEncodeC(data, max);
  return std::make_pair(std::move(pair.first),
                        std::make_pair(std::move(pair.second),
                                       std::move(max)));
}

/// \fn AdaptiveDecodeC(const std::vector<std::uint8_t>& data,
///                    std::size_t original_size,
///                    const T& max)
/// \brief RangeCoder Adaptive Decode Function.
///        To solve the zero-frequency-problem, use the Method C.
/// \param[in] data sequence
/// \param[in] original_size length of the original sequence
/// \param[in] max maximum value of the original sequence
/// \return original sequence as \c std::vector<T>
template <typename T>
auto AdaptiveDecodeC(const std::vector<std::uint8_t>& data,
                     std::size_t original_size,
                     const T& max) {
  auto A = std::size_t(max + 1);
  std::vector<unsigned_integer_t> sum_freq(A, 0), freq(A, 0);
  std::vector<unsigned_integer_t> sum_nfreq(A, 0), nfreq(A, 1);
  unsigned_integer_t sum = 0;
  for (std::size_t i = 0; i < sum_nfreq.size(); i++) {
    sum_nfreq[i] = sum;
    sum += nfreq[i];
  }
  std::size_t incorrect_sum_freq_min = 0, u = 0;
  std::vector<T> ret;
  auto&& cont = decode_init<unsigned_integer_size, T>(data);
  for (std::size_t i = 0; i < original_size; i++) {
    auto n = i + u;
    auto decode = decode_partial_fetch(cont, freq, i, n);
    auto d = std::size_t(decode.first);
    if (!decode.second || i == 0) {
      if (i != 0) {
        cont = decode_process(std::move(cont), data, i, u, n);
      }
      auto Td = decode_fetch(cont, nfreq, A - u);
      d = std::size_t(Td);
      cont = decode_process(std::move(cont), data, sum_nfreq[d], 1, A - u);
      nfreq[d] = 0;
      for (auto j = d + 1; j < sum_nfreq.size(); j++) {
        sum_nfreq[j] = sum_nfreq[j - 1] + nfreq[j - 1];
      }
      ret.push_back(Td);
      u++;
    } else {
      if (incorrect_sum_freq_min <= d) {
        if (incorrect_sum_freq_min == 0) {
          incorrect_sum_freq_min = 1;
        }
        for (auto j = incorrect_sum_freq_min; j <= d; j++) {
          sum_freq[j] = sum_freq[j - 1] + freq[j - 1];
        }
        incorrect_sum_freq_min = d + 1;
      }
      cont = decode_process(std::move(cont), data, sum_freq[d], freq[d], n);
      ret.push_back(decode.first);
    }
    freq[d]++;
    if (d < incorrect_sum_freq_min) {
      incorrect_sum_freq_min = d;
    }
  }
  return ret;
}

/// \fn AdaptiveDecodeC(const std::pair<
///                            std::vector<std::uint8_t>,
///                            std::pair<std::size_t, T>>& pair)
/// \brief RangeCoder Adaptive Decode Function.
///        To solve the zero-frequency-problem, use the Method C.
/// \param[in] pair \c std::pair of sequence as std::vector<std::uint8_t> and
///            \c std::pair of length of the original sequence as
///            \c std::size_t and maximum value of the original sequence
/// \return original sequence as \c std::vector<T>
template <typename T>
auto AdaptiveDecodeC(const std::pair<std::vector<std::uint8_t>,
                           std::pair<std::size_t, T>>& tuple) {
  return AdaptiveDecodeC(tuple.first, tuple.second.first, tuple.second.second);
}

/// \fn AdaptiveEncodeD(const std::vector<T>& data, const T& max)
/// \brief RangeCoder Adaptive Encode Function.
///        To solve the zero-frequency-problem, use the Method D.
/// \param[in] data sequence
/// \param[in] max maximum value of the data
/// \return \c std::pair of sequence as \c std::vector<std::uint8_t> and
///         length of the data sequence as \c std::size_t
template <typename T>
auto AdaptiveEncodeD(const std::vector<T>& data, const T& max) {
  auto A = std::size_t(max + 1);
  std::vector<unsigned_integer_t> sum_freq(A, 0), freq(A, 0);
  std::vector<unsigned_integer_t> sum_nfreq(A, 0), nfreq(A, 1);
  unsigned_integer_t sum = 0;
  for (std::size_t i = 0; i < sum_nfreq.size(); i++) {
    sum_nfreq[i] = sum;
    sum += nfreq[i];
  }
  std::size_t incorrect_sum_freq_min = 0, u = 0;
  auto&& cont = encode_init<unsigned_integer_size>();
  for (std::size_t i = 0; i < data.size(); i++) {
    auto d = std::size_t(data[i]);
    if (freq[d] == 0) {
      if (i != 0) {
        cont = encode_process(std::move(cont), i * 2 - u, u, i * 2);
      }
      cont = encode_process(std::move(cont), sum_nfreq[d], 1, A - u);
      nfreq[d] = 0;
      for (auto j = d + 1; j < sum_nfreq.size(); j++) {
        sum_nfreq[j] = sum_nfreq[j - 1] + nfreq[j - 1];
      }
      u++;
      freq[d]--;
    } else {
      if (incorrect_sum_freq_min <= d) {
        if (incorrect_sum_freq_min == 0) {
          incorrect_sum_freq_min = 1;
        }
        for (auto j = incorrect_sum_freq_min; j <= d; j++) {
          sum_freq[j] = sum_freq[j - 1] + freq[j - 1];
        }
        incorrect_sum_freq_min = d + 1;
      }
      cont = encode_process(std::move(cont), sum_freq[d], freq[d], i * 2);
    }
    freq[d] += 2;
    if (d < incorrect_sum_freq_min) {
      incorrect_sum_freq_min = d;
    }
  }
  return std::make_pair(encode_finish(std::move(cont)), data.size());
}

/// \fn AdaptiveEncodeD(const std::vector<T>& data)
/// \brief RangeCoder Adaptive Encode Function.
///        To solve the zero-frequency-problem, use the Method D.
/// \param[in] data sequence
/// \return \c std::pair of sequence as \c std::vector<std::uint8_t> and
///         \c std::pair of length of the original sequence as \c std::size_t
///         and maximum value of the original sequence as \c T
template <typename T>
auto AdaptiveEncodeD(const std::vector<T>& data) {
  T max{};
  for (std::size_t i = 0; i < data.size(); i++) {
    if (max < data[i]) {
      max = data[i];
    }
  }
  auto&& pair = AdaptiveEncodeD(data, max);
  return std::make_pair(std::move(pair.first),
                        std::make_pair(std::move(pair.second),
                                       std::move(max)));
}

/// \fn AdaptiveDecodeD(const std::vector<std::uint8_t>& data,
///                    std::size_t original_size,
///                    const T& max)
/// \brief RangeCoder Adaptive Decode Function.
///        To solve the zero-frequency-problem, use the Method D.
/// \param[in] data sequence
/// \param[in] original_size length of the original sequence
/// \param[in] max maximum value of the original sequence
/// \return original sequence as \c std::vector<T>
template <typename T>
auto AdaptiveDecodeD(const std::vector<std::uint8_t>& data,
                     std::size_t original_size,
                     const T& max) {
  auto A = std::size_t(max + 1);
  std::vector<unsigned_integer_t> sum_freq(A, 0), freq(A, 0);
  std::vector<unsigned_integer_t> sum_nfreq(A, 0), nfreq(A, 1);
  unsigned_integer_t sum = 0;
  for (std::size_t i = 0; i < sum_nfreq.size(); i++) {
    sum_nfreq[i] = sum;
    sum += nfreq[i];
  }
  std::size_t incorrect_sum_freq_min = 0, u = 0;
  std::vector<T> ret;
  auto&& cont = decode_init<unsigned_integer_size, T>(data);
  for (std::size_t i = 0; i < original_size; i++) {
    auto decode = decode_partial_fetch(cont, freq, i * 2 - u, i * 2);
    auto d = std::size_t(decode.first);
    if (!decode.second || i == 0) {
      if (i != 0) {
        cont = decode_process(std::move(cont), data, i * 2 - u, u, i * 2);
      }
      auto Td = decode_fetch(cont, nfreq, A - u);
      d = std::size_t(Td);
      cont = decode_process(std::move(cont), data, sum_nfreq[d], 1, A - u);
      nfreq[d] = 0;
      for (auto j = d + 1; j < sum_nfreq.size(); j++) {
        sum_nfreq[j] = sum_nfreq[j - 1] + nfreq[j - 1];
      }
      ret.push_back(Td);
      u++;
      freq[d]--;
    } else {
      if (incorrect_sum_freq_min <= d) {
        if (incorrect_sum_freq_min == 0) {
          incorrect_sum_freq_min = 1;
        }
        for (auto j = incorrect_sum_freq_min; j <= d; j++) {
          sum_freq[j] = sum_freq[j - 1] + freq[j - 1];
        }
        incorrect_sum_freq_min = d + 1;
      }
      cont = decode_process(std::move(cont), data, sum_freq[d], freq[d], i * 2);
      ret.push_back(decode.first);
    }
    freq[d] += 2;
    if (d < incorrect_sum_freq_min) {
      incorrect_sum_freq_min = d;
    }
  }
  return ret;
}

/// \fn AdaptiveDecodeD(const std::pair<
///                            std::vector<std::uint8_t>,
///                            std::pair<std::size_t, T>>& pair)
/// \brief RangeCoder Adaptive Decode Function.
///        To solve the zero-frequency-problem, use the Method D.
/// \param[in] pair \c std::pair of sequence as std::vector<std::uint8_t> and
///            \c std::pair of length of the original sequence as
///            \c std::size_t and maximum value of the original sequence
/// \return original sequence as \c std::vector<T>
template <typename T>
auto AdaptiveDecodeD(const std::pair<std::vector<std::uint8_t>,
                           std::pair<std::size_t, T>>& tuple) {
  return AdaptiveDecodeD(tuple.first, tuple.second.first, tuple.second.second);
}

}  // namespace RangeCoder
}  // namespace ResearchLibrary

#endif  // INCLUDES_RANGE_CODER_H_
