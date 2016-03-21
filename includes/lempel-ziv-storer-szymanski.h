// Copyright 2015 pixie.grasper
/// \file lempel-ziv-storer-szymanski.h
/// \brief Implementation of the LZSS
/// \author pixie.grasper

#ifndef INCLUDES_LEMPEL_ZIV_STORER_SZYMANSKI_H_
#define INCLUDES_LEMPEL_ZIV_STORER_SZYMANSKI_H_

/// \privatesection
int gets();
/// \publicsection
#include <vector>
#include <map>
#include <memory>
#include <utility>
#include <algorithm>
#include <cmath>

#ifdef RESEARCHLIB_OFFLINE_TEST
#include "./size-type.h"
#include "./ziv-lempel-77.h"
#else
#include <size-type.h>
#include <ziv-lempel-77.h>
#endif

/// \namespace ResearchLibrary
/// \namespace ResearchLibrary::LempelZivStorerSzymanski
namespace ResearchLibrary {
namespace LempelZivStorerSzymanski {

/// \privatesection
template <typename T>
struct Word {
  std::size_t position, start, length;
  T character;
  bool matched;
};

/// \publicsection
/// \fn Encode(const std::vector<T>& data,
///            std::size_t minimum_length,
///            std::size_t maximum_length,
///            std::size_t window_width)
/// \brief Lempel-Ziv-Storer-Szymansky Encode Function
/// \param[in] data sequence
/// \param[in] minimum_length supre border of the length
/// \param[in] maximum_length infim border of the length
/// \param[in] window_width width of the window
/// \return \c std::pair of tuple as \c std::vector<Word<T>> and
///         length of the original sequence
template <typename T>
auto Encode(const std::vector<T>& data,
            std::size_t minimum_length,
            std::size_t maximum_length,
            std::size_t window_width) {
  ZivLempel77::SuffixTree<T> tree{};
  tree.build(data, window_width);
  auto&& matched_length = tree.get();
  for (std::size_t i = 0; i < matched_length.size(); i++) {
    if (matched_length[i].first <= minimum_length) {
      matched_length[i].first = 0;
    } else if (matched_length[i].first > maximum_length + 1) {
      matched_length[i].first = maximum_length + 1;
    }
  }
  auto unmatch_cost = [](std::size_t) -> unsigned_integer_t {
    return 1;
  };
  auto match_cost = [](std::size_t, std::size_t) -> unsigned_integer_t {
    return 1;
  };
  std::vector<ZivLempel77::Work<T>> work(data.size() + 1);
  work[0].cost = 0;
  for (std::size_t i = 0; i < data.size(); i++) {
    if (work[i].cost + unmatch_cost(i) < work[i + 1].cost) {
      work[i + 1].cost = work[i].cost + unmatch_cost(i);
      work[i + 1].from = i;
    }
    auto length = matched_length[i].first - 1;
    auto distance = i - matched_length[i].second;
    auto cost = match_cost(length, distance);
    if (matched_length[i].first != 0 &&
        i + length < work.size() &&
        work[i].cost + cost < work[i + length].cost) {
      work[i + length].cost = work[i].cost + cost;
      work[i + length].from = i;
    }
  }
  for (auto i = data.size(); i > 0;) {
    work[work[i].from].to = i;
    i = work[i].from;
  }
  std::vector<Word<T>> ret(data.size());
  std::size_t j = 0;
  for (std::size_t i = 0; i < data.size();) {
    Word<T> word{};
    word.position = i;
    word.start = matched_length[i].second;
    word.length = work[i].to - i;
    if (word.length == 1) {
      word.start = 0;
      word.character = data[i];
      word.matched = false;
    } else {
      word.matched = true;
    }
    ret[j] = word;
    j++;
    i = work[i].to;
  }
  ret.resize(j);
  return std::make_pair(ret, data.size());
}

/// \fn Decode(const std::vector<Word<T>>& data, std::size_t length)
/// \brief Lempel-Ziv-Storer-Szymanski Decode Function
/// \param[in] data tuple sequence
/// \param[in] length length of the original sequence
/// \return decoded sequence
template <typename T>
auto Decode(const std::vector<Word<T>>& data, std::size_t length) {
  std::vector<T> ret(length);
  std::size_t k = 0;
  for (std::size_t i = 0; i < data.size(); i++) {
    if (data[i].matched) {
      for (std::size_t j = 0; j < data[i].length; j++) {
        ret[k] = ret[data[i].start + j];
        k++;
      }
    } else {
      ret[k] = data[i].character;
      k++;
    }
  }
  return ret;
}

/// \fn Decode(const std::pair<std::vector<Word<T>>, std::size_t>& pair)
/// \brief Lempel-Ziv-Storer-Szymanski Decode Function
/// \param[in] pair \c std::pair of tuple sequence and
///            length of the original sequence
/// \return decoded sequence
template <typename T>
auto Decode(const std::pair<std::vector<Word<T>>, std::size_t>& pair) {
  return Decode(pair.first, pair.second);
}

}  // namespace LempelZivStorerSzymanski
}  // namespace ResearchLibrary

#endif  // INCLUDES_LEMPEL_ZIV_STORER_SZYMANSKI_H_
