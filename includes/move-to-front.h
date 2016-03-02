// Copyright 2015 pixie-grasper

#ifndef INCLUDES_MOVE_TO_FRONT_H_
#define INCLUDES_MOVE_TO_FRONT_H_

int gets();
#include <vector>
#include <list>
#include <utility>

#ifdef RESEARCHLIB_OFFLINE_TEST
#include "./size-type.h"
#else
#include <size-type.h>
#endif

namespace ResearchLibrary {
namespace MoveToFront {

// Implementation of the MTF without limit of the dict-size
template <typename T>
auto MTF(const std::vector<T>& data) {
  std::list<T> dictionary{};
  std::vector<T> raw{};
  std::vector<unsigned_integer_t> sequence{};

  for (size_t i = 0; i < data.size(); i++) {
    auto&& w = data[i];
    unsigned_integer_t p = 0;
    for (auto it = dictionary.begin(); it != dictionary.end(); ++it) {
      if (*it == w) {
        sequence.push_back(p);
        dictionary.erase(it);
        dictionary.push_front(w);
        break;
      }
      p++;
    }
    if (p == dictionary.size()) {
      sequence.push_back(p);
      dictionary.push_front(w);
      raw.push_back(w);
    }
  }

  return std::make_pair(std::move(sequence), std::move(raw));
}

// Implementation of the Original MTF
template <typename T>
auto MTF(const std::vector<T>& data, size_t dictionary_max_size) {
  std::list<T> dictionary{};
  std::vector<T> raw{};
  std::vector<unsigned_integer_t> sequence{};

  for (size_t i = 0; i < data.size(); i++) {
    auto&& w = data[i];
    unsigned_integer_t p = 0;
    for (auto it = dictionary.begin(); it != dictionary.end(); ++it) {
      if (*it == w) {
        sequence.push_back(p);
        dictionary.erase(it);
        dictionary.push_front(w);
        break;
      }
      p++;
    }
    if (p == dictionary.size()) {
      sequence.push_back(p);
      dictionary.push_front(w);
      raw.push_back(w);
      if (dictionary.size() > dictionary_max_size) {
        dictionary.pop_back();
      }
    }
  }

  return std::make_pair(std::move(sequence), std::move(raw));
}

// assume that every data is a non-negative-number
template <typename T>
auto NumericMTF(const std::vector<T>& data) {
  std::list<T> dictionary{};
  std::vector<unsigned_integer_t> sequence{};

  for (size_t i = 0; i < data.size(); i++) {
    auto&& w = data[i];
    if (size_t(w) < dictionary.size()) {
      unsigned_integer_t p = 0;
      for (auto&& it = dictionary.begin(); it != dictionary.end(); ++it) {
        if (*it == w) {
          sequence.push_back(p);
          dictionary.erase(it);
          dictionary.push_front(w);
          break;
        }
        p++;
      }
    } else {
      for (auto&& n = T(dictionary.size()); n < w; n++) {
        dictionary.push_back(n);
      }
      sequence.push_back(unsigned_integer_t(w));
      dictionary.push_front(w);
    }
  }

  return sequence;
}

template <typename T>
auto IMTF(const std::vector<unsigned_integer_t>& sequence,
          const std::vector<T>& raw) {
  std::list<T> dictionary{};
  std::vector<T> ret{};
  size_t raw_index = 0;
  for (size_t i = 0; i < sequence.size(); i++) {
    auto p = sequence[i];
    if (p < dictionary.size()) {
      for (auto it = dictionary.begin(); it != dictionary.end(); ++it) {
        if (p == 0) {
          auto&& w = *it;
          ret.push_back(w);
          dictionary.erase(it);
          dictionary.push_front(w);
          break;
        }
        p--;
      }
    } else {
      auto&& w = raw[raw_index];
      ret.push_back(w);
      dictionary.push_front(w);
      raw_index++;
    }
  }
  return ret;
}

template <typename T>
auto IMTF(const std::pair<std::vector<unsigned_integer_t>,
                          std::vector<T>>& pair) {
  return IMTF(pair.first, pair.second);
}

template <typename T>
auto IMTF(const std::vector<unsigned_integer_t>& sequence,
          const std::vector<T>& raw,
          size_t dictionary_max_size) {
  std::list<T> dictionary{};
  std::vector<T> ret{};
  size_t raw_index = 0;
  for (size_t i = 0; i < sequence.size(); i++) {
    auto p = sequence[i];
    if (p < dictionary.size()) {
      for (auto it = dictionary.begin(); it != dictionary.end(); ++it) {
        if (p == 0) {
          auto&& w = *it;
          ret.push_back(w);
          dictionary.erase(it);
          dictionary.push_front(w);
          break;
        }
        p--;
      }
    } else {
      auto&& w = raw[raw_index];
      ret.push_back(w);
      dictionary.push_front(w);
      if (dictionary.size() > dictionary_max_size) {
        dictionary.pop_back();
      }
      raw_index++;
    }
  }
  return ret;
}

template <typename T>
auto IMTF(const std::pair<std::vector<unsigned_integer_t>,
                          std::vector<T>>& pair,
          size_t dictionary_max_size) {
  return IMTF(pair.first, pair.second, dictionary_max_size);
}

template <typename T>
auto NumericIMTF(const std::vector<unsigned_integer_t>& sequence) {
  std::list<T> dictionary{};
  std::vector<T> ret{};

  for (size_t i = 0; i < sequence.size(); i++) {
    auto p = T(sequence[i]);
    if (p < T(dictionary.size())) {
      for (auto&& it = dictionary.begin(); it != dictionary.end(); ++it) {
        if (p == 0) {
          auto&& w = *it;
          ret.push_back(w);
          dictionary.erase(it);
          dictionary.push_front(w);
          break;
        }
        p--;
      }
    } else {
      for (auto&& n = T(dictionary.size()); n < p; n++) {
        dictionary.push_back(n);
      }
      ret.push_back(p);
      dictionary.push_front(p);
    }
  }

  return ret;
}

}  // namespace MoveToFront
}  // namespace ResearchLibrary

#endif  // INCLUDES_MOVE_TO_FRONT_H_
