// Copyright 2015 pixie.grasper
/// \file deflate.h
/// \brief Implementation of the Deflate
/// \author pixie.grasper

#ifndef INCLUDES_DEFLATE_H_
#define INCLUDES_DEFLATE_H_

/// \privatesection
int gets();
/// \publicsection
#include <vector>
#include <array>
#include <map>
#include <memory>
#include <utility>
#include <algorithm>
#include <cmath>

#ifdef RESEARCHLIB_OFFLINE_TEST
#include "./size-type.h"
#include "./bit-byte-converter.h"
#include "./huffman-coding.h"
#include "./ziv-lempel-77.h"
#include "./lempel-ziv-storer-szymanski.h"
#else
#include <size-type.h>
#include <bit-byte-converter.h>
#include <huffman-coding.h>
#include <ziv-lempel-77.h>
#include <lempel-ziv-storer-szymanski.h>
#endif

/// \namespace ResearchLibrary
/// \namespace ResearchLibrary::Deflate
namespace ResearchLibrary {
namespace Deflate {

/// \privatesection
struct DistanceCode {
  std::size_t code, bits, distance, true_distance;
};

template <typename T>
std::size_t lcode_from_word(LempelZivStorerSzymanski::Word<T> word) {
  if (word.matched) {
    if (word.length <= 10) {
      return word.length + 254;
    } else if (word.length <= 18) {
      return (word.length - 11) / 2 + 265;
    } else if (word.length <= 34) {
      return (word.length - 19) / 4 + 269;
    } else if (word.length <= 66) {
      return (word.length - 35) / 8 + 273;
    } else if (word.length <= 130) {
      return (word.length - 67) / 16 + 277;
    } else if (word.length <= 257) {
      return (word.length - 131) / 32 + 281;
    } else {
      return 285;
    }
  } else {
    return word.character;
  }
}

template <typename = int>
auto distance_to_code(std::size_t distance) {
  DistanceCode ret{};
  ret.true_distance = distance;
  if (distance <= 4) {
    ret.code = distance - 1;
    ret.bits = 0;
    ret.distance = 0;
  } else {
    auto group = static_cast<std::size_t>(std::ceil(std::log2(distance)));
    std::size_t min = (1 << group) - (1 << (group - 1)) + 1;
    std::size_t border = (1 << group) - (1 << (group - 2));
    ret.bits = group - 2;
    if (distance <= border) {
      ret.code = group * 2 - 2;
      ret.distance = distance - min;
    } else {
      ret.code = group * 2 - 1;
      ret.distance = distance - border - 1;
    }
  }
  return ret;
}

template <typename = int>
auto Encode(const std::vector<std::uint8_t>& source) {
  // build lempel-ziv-tree
  std::size_t current_size = size_type<sizeof(std::size_t)>::max;
  ZivLempel77::SuffixTree<std::uint8_t> tree{};
  tree.build(source, 32768);
  // trunk mathcing length
  auto&& matched_length = tree.get();
  for (std::size_t i = 0; i < matched_length.size(); i++) {
    for (auto it = matched_length[i].begin();
              it != matched_length[i].end();
              ++it) {
      if (it->first <= 3) {
        it->first = 0;
      } else if (it->first > 258 + 1) {
        it->first = 258 + 1;
      }
    }
  }
  // initialize literal/distance table as fixed huffman coding
  std::vector<std::size_t> literal_length_table(288);
  for (std::size_t i = 0; i <= 143; i++) {
    literal_length_table[i] = 8;
  }
  for (std::size_t i = 144; i <= 255; i++) {
    literal_length_table[i] = 9;
  }
  for (std::size_t i = 256; i <= 279; i++) {
    literal_length_table[i] = 7;
  }
  for (std::size_t i = 280; i <= 287; i++) {
    literal_length_table[i] = 8;
  }
  std::vector<std::size_t> distance_length_table(30);
  for (std::size_t i = 0; i <= 29; i++) {
    distance_length_table[i] = 5;
  }
  constexpr std::array<std::size_t, 259> length_to_code
    = {{0, 0, 0, 257, 258, 259, 260, 261, 262, 263, 264,
        265, 265, 266, 266, 267, 267, 268, 268,
        269, 269, 269, 269, 270, 270, 270, 270,
        271, 271, 271, 271, 272, 272, 272, 272,
        273, 273, 273, 273, 273, 273, 273, 273,
        274, 274, 274, 274, 274, 274, 274, 274,
        275, 275, 275, 275, 275, 275, 275, 275,
        276, 276, 276, 276, 276, 276, 276, 276,
        277, 277, 277, 277, 277, 277, 277, 277,
        277, 277, 277, 277, 277, 277, 277, 277,
        278, 278, 278, 278, 278, 278, 278, 278,
        278, 278, 278, 278, 278, 278, 278, 278,
        279, 279, 279, 279, 279, 279, 279, 279,
        279, 279, 279, 279, 279, 279, 279, 279,
        280, 280, 280, 280, 280, 280, 280, 280,
        280, 280, 280, 280, 280, 280, 280, 280,
        281, 281, 281, 281, 281, 281, 281, 281,
        281, 281, 281, 281, 281, 281, 281, 281,
        281, 281, 281, 281, 281, 281, 281, 281,
        281, 281, 281, 281, 281, 281, 281, 281,
        282, 282, 282, 282, 282, 282, 282, 282,
        282, 282, 282, 282, 282, 282, 282, 282,
        282, 282, 282, 282, 282, 282, 282, 282,
        282, 282, 282, 282, 282, 282, 282, 282,
        283, 283, 283, 283, 283, 283, 283, 283,
        283, 283, 283, 283, 283, 283, 283, 283,
        283, 283, 283, 283, 283, 283, 283, 283,
        283, 283, 283, 283, 283, 283, 283, 283,
        284, 284, 284, 284, 284, 284, 284, 284,
        284, 284, 284, 284, 284, 284, 284, 284,
        284, 284, 284, 284, 284, 284, 284, 284,
        284, 284, 284, 284, 284, 284, 284, 285}};
  constexpr std::array<std::size_t, 29> literal_extra_bits
    = {{0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
        1, 1, 2, 2, 2, 2, 3, 3, 3, 3,
        4, 4, 4, 4, 5, 5, 5, 5, 0}};
Encode_l1:
  // deside route
  std::vector<ZivLempel77::Work<std::size_t>> work(source.size() + 1);
  work[0].cost = 0;
  for (std::size_t i = 0; i < source.size(); i++) {
    auto unmatch_cost = literal_length_table[source[i]];
    if (work[i].cost + unmatch_cost < work[i + 1].cost) {
      work[i + 1].cost = work[i].cost + unmatch_cost;
      work[i + 1].from = i;
    }
    for (std::size_t j = 0; j < matched_length[i].size(); j++) {
      if (matched_length[i][j].first != 0) {
        auto length = matched_length[i][j].first - 1;
        auto lcode = length_to_code[length];
        auto dcode = distance_to_code(i - matched_length[i][j].second);
        auto cost = literal_length_table[lcode] +
                    literal_extra_bits[lcode - 257] +
                    distance_length_table[dcode.code] +
                    dcode.bits;
        if (i + length < work.size() &&
            work[i].cost + cost < work[i + length].cost) {
          work[i + length].cost = work[i].cost + cost;
          work[i + length].start = matched_length[i][j].second;
          work[i + length].from = i;
        }
      }
    }
  }
  for (auto i = source.size(); i > 0;) {
    work[work[i].from].to = i;
    i = work[i].from;
  }
  std::vector<LempelZivStorerSzymanski::Word<std::size_t>> lz;
  for (std::size_t i = 0; i < source.size();) {
    LempelZivStorerSzymanski::Word<std::size_t> word{};
    word.position = i;
    word.length = work[i].to - i;
    if (word.length == 1) {
      word.start = 0;
      word.character = source[i];
      word.matched = false;
    } else {
      word.start = work[work[i].to].start;
      word.matched = true;
    }
    lz.push_back(word);
    i = work[i].to;
  }
  // encode
  std::vector<std::size_t> literals{}, distances{};
  std::size_t max_literal = 257, max_distance = 1;
  for (std::size_t i = 0; i < lz.size(); i++) {
    auto lcode = lcode_from_word(lz[i]);
    if (max_literal < lcode) {
      max_literal = lcode;
    }
    if (lz[i].matched) {
      auto distance = lz[i].position - lz[i].start;
      auto code = distance_to_code(distance);
      if (max_distance < distance) {
        max_distance = distance;
      }
      literals.push_back(lcode);
      distances.push_back(code.code);
    } else {
      literals.push_back(lcode);
    }
  }
  literals.push_back(256);
  std::vector<std::size_t> codes{};
  codes.insert(codes.end(), literals.begin(), literals.end());
  codes.insert(codes.end(), distances.begin(), distances.end());
  auto length_map = HuffmanCoding::length_map_from_data(codes, 15);
  std::vector<std::size_t> pack
    = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
  std::vector<std::size_t> unpack
    = {3, 17, 15, 13, 11, 9, 7, 5, 4, 6, 8, 10, 12, 14, 16, 18, 0, 1, 2};

  auto literal_length_map = HuffmanCoding::length_map_from_data(literals, 15);
  auto literal_code_map = HuffmanCoding
                        ::length_map_to_code_map(literal_length_map);
  auto hlit = std::max_element(literal_length_map.begin(),
                               literal_length_map.end())->first - 257 + 1;
  auto distance_length_map = HuffmanCoding::length_map_from_data(distances, 15);
  auto distance_code_map = HuffmanCoding
                         ::length_map_to_code_map(distance_length_map);
  auto hdist = std::max_element(distance_length_map.begin(),
                                distance_length_map.end())->first - 1 + 1;
  std::vector<std::size_t> length_sequence_raw{};
  for (std::size_t i = 0; i < hlit + 257; i++) {
    length_sequence_raw.push_back(literal_code_map[i].first);
  }
  for (std::size_t i = 0; i < hdist + 1; i++) {
    length_sequence_raw.push_back(distance_code_map[i].first);
  }
  std::vector<std::size_t> length_sequence{};
  for (std::size_t i = 0; i < length_sequence_raw.size(); i++) {
    length_sequence.push_back(length_sequence_raw[i]);
  }
  std::size_t max_length_length_index = 0;
  for (std::size_t i = 0; i < length_sequence.size(); i++) {
    if (max_length_length_index < unpack[length_sequence[i]]) {
      max_length_length_index = unpack[length_sequence[i]];
    }
  }
  auto length_length_map = HuffmanCoding
                         ::length_map_from_data(length_sequence, 7);
  auto length_code_map = HuffmanCoding
                       ::length_map_to_code_map(length_length_map);
  auto hclen = max_length_length_index - 4 + 1;
  BitsToBytes<8> buffer{};
  buffer.put(1, 1);  // final
  buffer.put(2, 2);  // compressed with dynamic Huffman codes
  buffer.put(hlit, 5);
  buffer.put(hdist, 5);
  buffer.put(hclen, 4);
  for (std::size_t i = 0; i < hclen + 4; i++) {
    buffer.put(length_code_map[pack[i]].first, 3);
  }
  // literal/length and distance
  for (std::size_t i = 0; i < hlit + hdist + 258; i++) {
    auto l = length_sequence[i];
    buffer.rput(length_code_map[l].second,
                length_code_map[l].first);
  }
  // actual compressed data of the block
  std::vector<std::size_t> literal_extra_base
    = { 3,  4,  5,   6,   7,   8,   9,  10,  11, 13,
       15, 17, 19,  23,  27,  31,  35,  43,  51, 59,
       67, 83, 99, 115, 131, 163, 195, 227, 258};
  for (std::size_t i = 0; i < lz.size(); i++) {
    auto l = lcode_from_word(lz[i]);
    if (lz[i].matched) {
      auto distance = lz[i].position - lz[i].start;
      auto code = distance_to_code(distance);
      buffer.rput(literal_code_map[l].second, literal_code_map[l].first);
      if (literal_extra_bits[l - 257] != 0) {
        buffer.put(lz[i].length - literal_extra_base[l - 257],
                   literal_extra_bits[l - 257]);
      }
      buffer.rput(distance_code_map[code.code].second,
                  distance_code_map[code.code].first);
      if (code.bits != 0) {
        buffer.put(code.distance, code.bits);
      }
    } else {
      buffer.rput(literal_code_map[l].second, literal_code_map[l].first);
    }
  }
  buffer.rput(literal_code_map[256].second, literal_code_map[256].first);
  auto&& ret = buffer.seek_to_byte_boundary();
  if (current_size > ret.size()) {
    current_size = ret.size();
    for (std::size_t i = 0; i < literal_length_table.size(); i++) {
      literal_length_table[i] = literal_length_map[i];
    }
    for (std::size_t i = 0; i < distance_length_table.size(); i++) {
      distance_length_table[i] = distance_length_map[i];
    }
    goto Encode_l1;
  }
  return ret;
}

}  // namespace Deflate
}  // namespace ResearchLibrary

#endif  // INCLUDES_DEFLATE_H_
