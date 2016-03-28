// Copyright 2015 pixie.grasper
/// \file huffman-coding.h
/// \brief Implementation of the Huffman Coding
/// \author pixie.grasper

#ifndef INCLUDES_HUFFMAN_CODING_H_
#define INCLUDES_HUFFMAN_CODING_H_

/// \privatesection
int gets();
/// \publicsection
#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <memory>
#include <utility>
#include <queue>
#include <algorithm>

#ifdef RESEARCHLIB_OFFLINE_TEST
#include "./size-type.h"
#include "./bit-byte-converter.h"
#else
#include <size-type.h>
#include <bit-byte-converter.h>
#endif

/// \namespace ResearchLibrary
/// \namespace ResearchLibrary::HuffmanCoding
namespace ResearchLibrary {
namespace HuffmanCoding {

/// \privatesection
template <typename T>
auto make_heap_for_huffman_coding(std::vector<T>&& heap,
                                  unsigned_integer_t i,
                                  unsigned_integer_t b) {
  for (;;) {
    auto mi = i;
    auto l = 2 * i + 1;
    auto r = 2 * i + 2;
    if (l < b && heap[heap[l]] < heap[heap[i]]) {
      mi = l;
    }
    if (r < b && heap[heap[r]] < heap[heap[mi]]) {
      mi = r;
    }
    if (mi != i) {
      std::swap(heap[i], heap[mi]);
      i = mi;
      continue;
    }
    break;
  }
  return std::move(heap);
}

template <typename T>
auto length_map_to_code_map(const std::map<T, unsigned_integer_t>& length_map) {
  std::size_t max_length = 0;
  for (auto it = length_map.begin(); it != length_map.end(); ++it) {
    if (max_length < it->second) {
      max_length = it->second;
    }
  }

  // code-mapping algorithm from rfc1951
  std::map<T, std::pair<unsigned_integer_t, unsigned_integer_t>> code_map{};
  std::vector<unsigned_integer_t> bl_count(max_length + 1);
  for (auto it = length_map.begin(); it != length_map.end(); ++it) {
    bl_count[it->second]++;
  }
  std::vector<unsigned_integer_t> next_code(max_length + 1);
  unsigned_integer_t code = 0;
  for (unsigned_integer_t bits = 1; bits <= max_length; bits++) {
    code = (code + bl_count[bits - 1]) << 1;
    next_code[bits] = code;
  }
  for (auto it = length_map.begin(); it != length_map.end(); ++it) {
    auto ch = it->first;
    auto len = it->second;
    code_map[ch].first = len;
    code_map[ch].second = next_code[len];
    next_code[len]++;
  }
  return code_map;
}

template <std::size_t N>
size_type_t<N> mask(std::size_t n) {
  return (size_type_t<N>(1) << n) - 1;
}

template <typename T>
struct HuffmanNode {
  std::shared_ptr<HuffmanNode<T>> left, right;
  std::pair<T, unsigned_integer_t> value;
  unsigned_integer_t length;

  HuffmanNode() = default;

  explicit HuffmanNode(const std::pair<T, unsigned_integer_t>& value_)
      : left(nullptr), right(nullptr), value(value_), length(0) {
    return;
  }

  HuffmanNode(std::shared_ptr<HuffmanNode<T>>&& l,
              std::shared_ptr<HuffmanNode<T>>&& r)
      : left(std::move(l)),
        right(std::move(r)),
        value(std::make_pair(T{}, left->value.second + right->value.second)),
        length(0) {
    return;
  }

  void insert(unsigned_integer_t code_length,
              unsigned_integer_t code,
              const T& v) {
    if (code_length == 0) {
      value.first = v;
    } else {
      if (((code >> (code_length - 1)) & 1) == 1) {
        if (right == nullptr) {
          right = std::make_shared<HuffmanNode<T>>();
        }
        right->insert(code_length - 1, code, v);
      } else {
        if (left == nullptr) {
          left = std::make_shared<HuffmanNode<T>>();
        }
        left->insert(code_length - 1, code, v);
      }
    }
    return;
  }

  bool is_node() const {
    if (right != nullptr || left != nullptr) {
      return true;
    }
    return false;
  }
};

template <typename T>
auto length_map_from_data_numeric(const std::vector<T>& data,
                                  std::size_t max_length = 0) {
  auto max = data[0];
  for (std::size_t i = 1; i < data.size(); i++) {
    if (max < data[i]) {
      max = data[i];
    }
  }
  auto N = std::size_t(max) + 1;
  std::vector<unsigned_integer_t> freq(N);
  for (std::size_t i = 0; i < data.size(); i++) {
    freq[std::size_t(data[i])]++;
  }
  auto shift_width = 0;
length_map_from_data_numeric_l1:
  std::vector<unsigned_integer_t> freq_array(N * 2);
  for (std::size_t i = 0; i < N; i++) {
    freq_array[i] = N + i;
    freq_array[N + i] = freq[i];
    if (shift_width != 0 && freq[i] != 0) {
      auto mask = std::size_t(1) << (shift_width - 1);
      freq_array[N + i] = (freq_array[N + i] + mask) >> shift_width;
      if (freq_array[N + i] == 0) {
        freq_array[N + i] = 1;
      }
    }
  }
  for (std::size_t i = (N - 1) / 2; i + 1 >= 1; i--) {
    freq_array = make_heap_for_huffman_coding(std::move(freq_array), i, N);
  }

  auto h = N;
  while (h - 1 > 0) {
    auto m1 = freq_array[0];
    freq_array[0] = freq_array[h - 1];
    h--;
    freq_array = make_heap_for_huffman_coding(std::move(freq_array), 0, h);
    auto m2 = freq_array[0];
    freq_array[h] = freq_array[m1] + freq_array[m2];
    freq_array[0] = freq_array[m1] = freq_array[m2] = h;
    freq_array = make_heap_for_huffman_coding(std::move(freq_array), 0, h);
  }

  freq_array[1] = 0;
  for (std::size_t i = 3; i < freq_array.size(); i++) {
    freq_array[i] = freq_array[freq_array[i]] + 1;
  }

  std::map<T, unsigned_integer_t> length_map{};
  std::size_t length_max = 0;
  for (std::size_t i = 0; i < N; i++) {
    if (freq[i] != 0) {
      length_map[T(i)] = freq_array[N + i];
      if (length_max < length_map[T(i)]) {
        length_max = length_map[T(i)];
      }
    }
  }
  if (max_length != 0 && length_max > max_length) {
    shift_width++;
    goto length_map_from_data_numeric_l1;
  }
  return length_map;
}

template <typename T>
auto length_map_from_data(const std::vector<T>& data,
                          std::size_t max_length = 0) {
  // calculate frequency-map
  std::map<T, unsigned_integer_t> freq{};
  for (std::size_t i = 0; i < data.size(); i++) {
    freq[data[i]]++;
  }

  auto shift_width = 0;
length_map_from_data_l1:
  std::map<T, unsigned_integer_t> frequency_map{};
  for (auto it = freq.begin(); it != freq.end(); ++it) {
    auto value = it->second;
    if (shift_width != 0) {
      auto mask = std::size_t(1) << (shift_width - 1);
      value = (value + mask) >> shift_width;
      if (value == 0) {
        value = 1;
      }
    }
    frequency_map[it->first] = value;
  }

  // make the huffman-tree using priority-queue
  auto cmp = [](auto&& x, auto&& y) {
    return x->value.second > y->value.second;  // less to front
  };
  std::priority_queue<std::shared_ptr<HuffmanNode<T>>,
                      std::vector<std::shared_ptr<HuffmanNode<T>>>,
                      decltype(cmp)> queue(cmp);
  for (auto it = frequency_map.begin(); it != frequency_map.end(); ++it) {
    queue.push(std::make_shared<HuffmanNode<T>>(*it));
  }
  std::shared_ptr<HuffmanNode<T>> root = nullptr;
  for (;;) {
    auto p1 = queue.top();
    queue.pop();
    if (queue.empty()) {
      root = p1;
      break;
    }
    auto p2 = queue.top();
    queue.pop();
    auto p3 = std::make_shared<HuffmanNode<T>>(std::move(p1), std::move(p2));
    queue.push(p3);
  }

  // calculate code-length for all character
  std::set<std::shared_ptr<HuffmanNode<T>>> set;
  std::map<T, unsigned_integer_t> length_map;
  root->length = 0;
  set.insert(root);
  while (!set.empty()) {
    auto it = set.begin();
    auto p = *it;
    set.erase(it);
    if (p->left != nullptr && p->right != nullptr) {
      p->left->length = p->length + 1;
      set.insert(p->left);
      p->right->length = p->length + 1;
      set.insert(p->right);
    } else {
      if (max_length != 0 && p->length > max_length) {
        shift_width++;
        goto length_map_from_data_l1;
      }
      length_map[p->value.first] = p->length;
    }
  }

  return length_map;
}

/// \publicsection
/// \fn Encode(const std::vector<T>& data,
///            const std::map<T, std::pair<unsigned_integer_t,
///                                        unsigned_integer_t>>& code_map)
/// \brief Huffman Coding Encode Function
/// \param[in] data sequence
/// \param[in] code_map code-map as \c std::map<T, std::pair<unsigned_integer_t,
///            unsigned_integer_t>> that maps to \c std::pair of length of the
///            code and the code.
/// \return encoded sequence as std::vector<std::uint8_t>
template <typename T>
auto Encode(const std::vector<T>& data,
            const std::map<T, std::pair<unsigned_integer_t,
                                        unsigned_integer_t>>& code_map) {
  BitsToBytes<8> buffer{};
  for (std::size_t i = 0; i < data.size(); i++) {
    auto it = code_map.find(data[i]);
    if (it == code_map.end()) {
      std::cerr << "word not in the dictionary." << std::endl;
      continue;
    }
    auto pair = it->second;
    auto length = pair.first;
    auto code = pair.second;
    buffer.rput(code, length);
  }
  return buffer.seek_to_byte_boundary();
}

/// \fn Encode(const std::vector<T>& data)
/// \brief Huffman Coding Encode Function
/// \param[in] data sequence
/// \return \c std::pair of encoded sequence as std::vector<std::uint8_t> and
///         \c std::pair of length of the original sequence and
///         \c std::map<T,std::pair<unsigned_integer_t,unsigned_integer_t>>
///         that maps to \c std::pair of length of the code and the code
template <typename T>
auto Encode(const std::vector<T>& data) {
  auto&& length_map = length_map_from_data(data);
  auto&& code_map = length_map_to_code_map(length_map);
  return std::make_pair(Encode(data, code_map),
         std::make_pair(data.size(), code_map));
}

/// \fn NumericEncode(const std::vector<T>& data,
///                   const std::map<T, unsigned_integer_t>& length_map)
/// \brief Huffman Coding Encode Function;
///        assume that every data is non-negative number
/// \param[in] data sequence
/// \param[in] length_map character maps to code length
/// \return \c std::pair of encoded sequence as \c std::vector<std::uint8_t> and
///         \c std::pair of length of the original sequence and
///         code-map as \c std::map<T, std::pair<unsigned_integer_t,
///         unsigned_integer_t>> that maps to \c std::pair of length of the
///         code and the code.
template <typename T>
auto NumericEncode(const std::vector<T>& data,
                   const std::map<T, unsigned_integer_t>& length_map) {
  auto&& code_map = length_map_to_code_map(length_map);
  return std::make_pair(Encode(data, code_map),
         std::make_pair(data.size(), code_map));
}

/// \fn NumericEncode(const std::vector<T>& data)
/// \brief Huffman Coding Encode Function;
///        assume that every data is non-negative-number
/// \param[in] data sequence
/// \return \c std::pair of encoded sequence as \c std::vector<std::uint8_t> and
///         \c std::pair of length of the original sequence and
///         length map as \c std::map<T,unsigned_integer_t>
template <typename T>
auto NumericEncode(const std::vector<T>& data) {
  auto length_map = length_map_from_data_numeric(data);
  return std::make_pair(NumericEncode(data, length_map).first,
         std::make_pair(data.size(), length_map));
}

/// \fn Decode(const std::vector<std::uint8_t>& data,
///            std::size_t length,
///            const std::map<T, std::pair<unsigned_integer_t,
///                                        unsigned_integer_t>>& code_map)
/// \brief Huffman Coding Decode Function
template <typename T>
auto Decode(const std::vector<std::uint8_t>& data,
            std::size_t length,
            const std::map<T, std::pair<unsigned_integer_t,
                                        unsigned_integer_t>>& code_map) {
  auto root = std::make_shared<HuffmanNode<T>>();
  for (auto it = code_map.begin(); it != code_map.end(); ++it) {
    root->insert(it->second.first, it->second.second, it->first);
  }
  BytesToBits<8> buffer(data);
  std::vector<T> ret(length);
  for (std::size_t i = 0; i < length; i++) {
    auto current_node = root;
    while (current_node->is_node()) {
      auto bit = buffer.get(1);
      if (bit == 0) {
        current_node = current_node->left;
      } else {
        current_node = current_node->right;
      }
    }
    ret[i] = current_node->value.first;
  }
  return ret;
}

/// \fn Decode(const std::vector<std::uint8_t>& data,
///            std::size_t length,
///            const std::map<T, unsigned_integer_t>& length_map)
/// \brief Huffman Coding Decode Function
template <typename T>
auto Decode(const std::vector<std::uint8_t>& data,
            std::size_t length,
            const std::map<T, unsigned_integer_t>& length_map) {
  auto&& code_map = length_map_to_code_map(length_map);
  return Decode(data, length, code_map);
}

/// \fn Decode(const std::pair<std::vector<std::uint8_t>,
///                  std::pair<std::size_t,
///                  std::map<T, std::pair<unsigned_integer_t,
///                                        unsigned_integer_t>>>>& tuple)
/// \brief Huffman Coding Decode Function
template <typename T>
auto Decode(const std::pair<std::vector<std::uint8_t>,
                  std::pair<std::size_t,
                  std::map<T, std::pair<unsigned_integer_t,
                                        unsigned_integer_t>>>>& tuple) {
  return Decode(tuple.first, tuple.second.first, tuple.second.second);
}

/// \fn Decode(const std::pair<std::vector<std::uint8_t>,
///                  std::pair<std::size_t,
///                  std::map<T, unsigned_integer_t>>>& tuple)
/// \brief Huffman Coding Decode Funciotn
template <typename T>
auto Decode(const std::pair<std::vector<std::uint8_t>,
                  std::pair<std::size_t,
                  std::map<T, unsigned_integer_t>>>& tuple) {
  return Decode(tuple.first, tuple.second.first, tuple.second.second);
}

}  // namespace HuffmanCoding
}  // namespace ResearchLibrary

#endif  // INCLUDES_HUFFMAN_CODING_H_

