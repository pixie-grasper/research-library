// Copyright 2015 pixie.grasper

#ifndef RANGE_CODER_H_
#define RANGE_CODER_H_

#include <stdint.h>

int gets();
#include <vector>
#include <map>
#include <utility>

#include "./size-type.h"

namespace ResearchLibrary {
namespace RangeCoder {

template <size_t N>
auto mulhi(size_type_t<N> u, size_type_t<N> v) {
  return size_type_t<N>((size_type_t<2 * N>(u) * size_type_t<2 * N>(v)) >> N);
}

template <>
auto mulhi<8>(size_type_t<8> u, size_type_t<8> v) {
  auto u0 = u & 0xffffffff, u1 = u >> 32;
  auto v0 = v & 0xffffffff, v1 = v >> 32;
  auto w0 = u0 * v0;
  auto t = u1 * v0 + (w0 >> 32);
  auto w1 = t & 0xffffffff, w2 = t >> 32;
  w1 = u0 * v1 + w1;
  return u1 * v1 + w2 + (w1 >> 32);
}

template <size_t N>
auto idiv(size_type_t<N> x, size_type_t<N> z) {
  return size_type_t<N>((size_type_t<2 * N>(x) << N) / z);
}

template <>
auto idiv<8>(size_type_t<8> x, size_type_t<8> z) {
  size_type_t<8> y = 0;
  for (auto i = 0; i < 64; i++) {
    auto t = false;
    if (x & ~(UINT64_MAX >> 1)) {
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

template <size_t N>
struct EncoderContinuation {
  std::vector<uint8_t> buffer;
  size_type_t<N> low, range;
};

template <size_t N>
auto encode_init() {
  EncoderContinuation<N> cont{};
  cont.range = size_type_t<N>(UINT64_MAX);
  return cont;
}

template <size_t N>
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

template <size_t N>
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

template <size_t N, typename T>
struct DecoderContinuation {
  size_type_t<N> low, range, data, index;
  T buffer;
};

template <size_t N, typename T>
auto decode_init(const std::vector<uint8_t>& data) {
  DecoderContinuation<N, T> cont{};
  cont.range = size_type_t<N>(UINT64_MAX);
  for (size_t i = 0; i < N; i++) {
    if (i < data.size()) {
      cont.data = (cont.data << 8) + data[i];
    } else {
      cont.data <<= 8;
    }
  }
  cont.index = 8;
  return cont;
}

template <size_t N, typename T>
auto decode_process(DecoderContinuation<N, T>&& cont,
                    const std::vector<uint8_t>& data,
                    const std::vector<std::pair<T, unsigned_integer_t>>& freq,
                    const std::vector<unsigned_integer_t>& sum_freq) {
  auto sum = sum_freq.back() + freq.back().second;
  auto ch_in = mulhi<N>(idiv<N>(cont.data - cont.low, cont.range), sum);
  size_t left = 0, right = freq.size() - 1;
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
  for (size_t i = 0; i < data.size(); i++) {
    auto low = idiv<unsigned_integer_size>(sum_freq[data[i]], sum);
    auto range = idiv<unsigned_integer_size>(freq_map[data[i]], sum);
    cont = encode_process(std::move(cont), low, range);
  }
  return encode_finish(std::move(cont));
}

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
  return std::make_pair(std::move(encoded), std::move(freq));
}

template <typename T>
auto StaticDecode(const std::vector<uint8_t>& data,
                  const std::vector<std::pair<T, unsigned_integer_t>>& freq,
                  size_t original_size) {
  std::vector<unsigned_integer_t> sum_freq;
  unsigned_integer_t sum = 0;
  for (auto&& it = freq.begin(); it != freq.end(); ++it) {
    sum_freq.push_back(sum);
    sum += it->second;
  }
  std::vector<T> ret;
  auto&& cont = decode_init<unsigned_integer_size, T>(data);
  for (size_t i = 0; i < original_size; i++) {
    cont = decode_process(std::move(cont), data, freq, sum_freq);
    ret.push_back(cont.buffer);
  }
  return ret;
}

template <typename T>
using static_decode_1st_arg_t =
std::pair<std::vector<uint8_t>, std::vector<std::pair<T, unsigned_integer_t>>>;

template <typename T>
auto StaticDecode(const static_decode_1st_arg_t<T>& pair,
                  size_t original_size) {
  return StaticDecode(pair.first, pair.second, original_size);
}

}  // namespace RangeCoder
}  // namespace ResearchLibrary

#endif  // RANGE_CODER_H_
