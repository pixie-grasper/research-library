// Copyright 2015 pixie-grasper
/// \file move-to-front.h
/// \brief Implementation of the MTF
/// \author pixie.grasper

#ifndef INCLUDES_MOVE_TO_FRONT_H_
#define INCLUDES_MOVE_TO_FRONT_H_

/// \privatesection
int gets();
/// \publicsection
#include <vector>
#include <list>
#include <utility>

#ifdef RESEARCHLIB_OFFLINE_TEST
#include "./size-type.h"
#else
#include <size-type.h>
#endif

/// \namespace ResearchLibrary
/// \namespace ResearchLibrary::MoveToFront
namespace ResearchLibrary {
namespace MoveToFront {

/// \fn MTF(const std::vector<T>& data)
/// \brief Move-to-Front Function without limit size of the dictionary
/// \param[in] data sequence
/// \return std::pair of MTF-ed sequence and dictionary as std::list<T>
template <typename T>
auto MTF(const std::vector<T>& data) {
  std::list<T> dictionary{};
  std::vector<T> raw{};
  std::vector<unsigned_integer_t> sequence{};

  for (std::size_t i = 0; i < data.size(); i++) {
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

/// \fn MTF(const std::vector<T>& data, std::size_t dictionary_max_size)
/// \brief Original Move-to-Front Function
/// \param[in] data sequence
/// \param[in] dictionary_max_size maximum size of the dictionary
/// \return std::pair of MTF-ed sequence and dictionary as std::list<T>
template <typename T>
auto MTF(const std::vector<T>& data, std::size_t dictionary_max_size) {
  std::list<T> dictionary{};
  std::vector<T> raw{};
  std::vector<unsigned_integer_t> sequence{};

  for (std::size_t i = 0; i < data.size(); i++) {
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

/// \fn NumericMTF(const std::vector<T>& data)
/// \brief Move-to-Front Function;
///        assume that every data is a non-negative-number
/// \param[in] data sequence
/// \return MTF-ed sequence
template <typename T>
auto NumericMTF(const std::vector<T>& data) {
  std::list<T> dictionary{};
  std::vector<unsigned_integer_t> sequence{};

  for (std::size_t i = 0; i < data.size(); i++) {
    auto&& w = data[i];
    if (std::size_t(w) < dictionary.size()) {
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

/// \fn IMTF(const std::vector<unsigned_integer_t>& sequence,
///          const std::vector<T>& raw)
/// \brief Inverse Move-to-Front Function
/// \param[in] sequence MTF-ed sequence
/// \param[in] raw Dictionary
/// \return sequence as std::vector<T>
template <typename T>
auto IMTF(const std::vector<unsigned_integer_t>& sequence,
          const std::vector<T>& raw) {
  std::list<T> dictionary{};
  std::vector<T> ret{};
  std::size_t raw_index = 0;
  for (std::size_t i = 0; i < sequence.size(); i++) {
    auto p = sequence[i];
    if (p < dictionary.size()) {
      for (auto it = dictionary.begin(); it != dictionary.end(); ++it) {
        if (p == 0) {
          auto&& w = *it;
          ret.push_back(w);
          dictionary.push_front(std::move(w));
          dictionary.erase(it);
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

/// \fn IMTF(const std::pair<std::vector<unsigned_integer_t>, std::vector<T>>&
///          pair)
/// \brief Inverse Move-to-Front Function
/// \param[in] pair std::pair of sequence MTF-ed sequence and dictionary
/// \return sequence as std::vector<T>
template <typename T>
auto IMTF(const std::pair<std::vector<unsigned_integer_t>,
                          std::vector<T>>& pair) {
  return IMTF(pair.first, pair.second);
}

/// \fn IMTF(const std::vector<unsigned_integer_t>& sequence,
///          const std::vector<T>& raw,
///          std::size_t dictionary_max_size)
/// \brief Original Inverse Move-to-Front Function
/// \param[in] sequence MTF-ed sequence
/// \param[in] raw Dictionary
/// \param[in] dictionary_max_size maximum size of the dictionary
/// \return sequence as std::vector<T>
template <typename T>
auto IMTF(const std::vector<unsigned_integer_t>& sequence,
          const std::vector<T>& raw,
          std::size_t dictionary_max_size) {
  std::list<T> dictionary{};
  std::vector<T> ret{};
  std::size_t raw_index = 0;
  for (std::size_t i = 0; i < sequence.size(); i++) {
    auto p = sequence[i];
    if (p < dictionary.size()) {
      for (auto it = dictionary.begin(); it != dictionary.end(); ++it) {
        if (p == 0) {
          auto&& w = *it;
          ret.push_back(w);
          dictionary.push_front(std::move(w));
          dictionary.erase(it);
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

/// \fn IMTF(const std::pair<std::vector<unsigned_integer_t>, std::vector<T>>&
///          pair,
///          std::size_t dictionary_max_size)
/// \brief Original Inverse Move-to-Front Function
/// \param[in] pair std::pair of sequence MTF-ed sequence and dictionary
/// \param[in] dictionary_max_size maximum size of the dictionary
/// \return sequence as std::vector<T>
template <typename T>
auto IMTF(const std::pair<std::vector<unsigned_integer_t>,
                          std::vector<T>>& pair,
          std::size_t dictionary_max_size) {
  return IMTF(pair.first, pair.second, dictionary_max_size);
}

/// \fn NumericIMTF(const std::vector<unsigned_integer_t>& sequence)
/// \brief Inverse Move-to-Front Function
/// \param[in] sequence sequence
/// \return sequence as std::vector<T>
template <typename T>
auto NumericIMTF(const std::vector<unsigned_integer_t>& sequence) {
  std::list<T> dictionary{};
  std::vector<T> ret{};

  for (std::size_t i = 0; i < sequence.size(); i++) {
    auto p = T(sequence[i]);
    if (p < T(dictionary.size())) {
      for (auto&& it = dictionary.begin(); it != dictionary.end(); ++it) {
        if (p == 0) {
          auto&& w = *it;
          ret.push_back(w);
          dictionary.push_front(std::move(w));
          dictionary.erase(it);
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
