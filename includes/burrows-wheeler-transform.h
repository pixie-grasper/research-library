// Copyright 2015 pixie.grasper
/// \file burrows-wheeler-transform.h
/// \brief Implementation of the BWT
/// \author pixie.grasper

#ifndef INCLUDES_BURROWS_WHEELER_TRANSFORM_H_
#define INCLUDES_BURROWS_WHEELER_TRANSFORM_H_

/// \privatesection
int gets();
/// \publicsection
#include <cstddef>
#include <vector>
#include <utility>
#include <algorithm>

/// \namespace ResearchLibrary
/// \namespace ResearchLibrary::BurrowsWheelerTransform
namespace ResearchLibrary {
namespace BurrowsWheelerTransform {

/// \privatesection
template <typename T>
bool ge(T x, T y) {
  if (y == 0) {
    return x >> (sizeof(T) * 8 - 1)? false: true;
  } else {
    return x >= y;
  }
}

template <typename T>
auto suffix_sort_for_BWT(const std::vector<T>& source) {
  constexpr auto SORTED_FLAG = std::size_t(1) << (sizeof(std::size_t) * 8 - 1);
  constexpr auto MASK = ~SORTED_FLAG;
  const auto N = source.size();
  std::vector<std::size_t> I(N), V(N);
  for (std::size_t i = 0; i < N; i++) {
    I[i] = i;
  }
  std::stable_sort(I.begin(), I.end(), [=](std::size_t x, std::size_t y) {
    return source[x] < source[y];
  });
  {
    auto last_character = source[I[0]];
    std::size_t g = 0;
    for (std::size_t i = N - 1; ge(i, decltype(i)(0)); i--) {
      if (last_character != source[I[i]]) {
        g = i;
        last_character = source[I[i]];
      }
      V[I[i]] = g;
    }
  }
  for (std::size_t i = 0; i < N;) {
    auto group = V[I[i]];
    if (i == group) {
      I[i] = SORTED_FLAG + 1;
    }
    i = group + 1;
  }
  std::size_t h = 1;
  while (h <= N) {
    std::size_t first_position = 0, group_start_index = 0;
    bool sorted_sequence = false;
    do {
      if ((I[first_position] & SORTED_FLAG) != 0) {
        if (!sorted_sequence) {
          group_start_index = first_position;
          sorted_sequence = true;
        }
        first_position += I[first_position] & MASK;
      } else {
        if (sorted_sequence) {
          I[group_start_index] = SORTED_FLAG
                               + (first_position - group_start_index);
          sorted_sequence = false;
        }
        auto last_position = V[I[first_position]];
        std::sort(&I[first_position],
                  &I[last_position] + 1,
                  [=](std::size_t x, std::size_t y) {
          return V[(x + h) % N] < V[(y + h) % N];
        });
        auto group = last_position;
        auto group_inv = V[(I[group] + h) % N];
        for (auto i = last_position; ge(i, first_position); i--) {
          auto i_inv = V[(I[i] + h) % N];
          if (first_position <= i_inv && i_inv < last_position) {
            i_inv = last_position;
          }
          if (group_inv != i_inv) {
            if (group == i + 1) {
              I[group] = SORTED_FLAG + 1;
            }
            group = i;
            group_inv = i_inv;
          }
          V[I[i]] = group;
        }
        if (group == first_position) {
          I[group] = SORTED_FLAG + 1;
        }
        first_position = last_position + 1;
      }
    } while (first_position < N);
    if (sorted_sequence) {
      I[group_start_index] = SORTED_FLAG + (first_position - group_start_index);
    }
    h <<= 1;
  }

  for (std::size_t i = 0; i < N; i++) {
    I[V[i]] = i;
  }
  return I;
}

/// \publicsection
/// \fn BWT(const std::vector<T>& source)
/// \brief Burrows-Wheeler Transform Function
/// \param[in] source sequence
/// \return \c std::pair of
///         sorted sequence as \c std::vector<T> and index as \c std::size_t
template <typename T>
auto BWT(const std::vector<T>& source) {
  if (source.size() == 0) {
    return std::make_pair(std::vector<T>(), static_cast<std::size_t>(0));
  } else if (source.size() == 1) {
    return std::make_pair(source, static_cast<std::size_t>(0));
  }
  auto&& suffix_array = suffix_sort_for_BWT(source);
  std::size_t index;
  std::vector<T> ret(source.size());
  for (std::size_t i = 0; i < suffix_array.size(); i++) {
    if (suffix_array[i] == 0) {
      index = i;
    }
    ret[i] = source[(suffix_array[i] - 1 + source.size()) % source.size()];
  }
  return std::make_pair(std::move(ret), index);
}

/// \fn IBWT(const std::vector<T>& source, std::size_t index)
/// \brief Inverse Burrows-Wheeler Transform Function
/// \param[in] source sorted sequence
/// \param[in] index The index
/// \return deconverted sequence as \c std::vector<T>
template <typename T>
auto IBWT(const std::vector<T>& source, std::size_t index) {
  const auto N = source.size();
  std::vector<std::size_t> buffer(N);
  for (std::size_t i = 0; i < N; i++) {
    buffer[i] = i;
  }
  std::stable_sort(buffer.begin(), buffer.end(), [=](auto&& x, auto&& y) {
    return source[x] < source[y];
  });
  std::vector<T> ret{};
  ret.reserve(N);
  for (auto i = buffer[index]; ret.size() < source.size(); i = buffer[i]) {
    ret.push_back(source[i]);
  }
  return ret;
}

/// \fn IBWT(const std::pair<std::vector<T>, std::size_t>& source)
/// \brief Inverse Burrows-Wheeler Transform Function
/// \param[in] source \c std::pair of sorted sequence as
///            \c std::vector<T> and index as \c std::size_t
/// \return deconverted sequence as \c std::vector<T>
template <typename T>
auto IBWT(const std::pair<std::vector<T>, std::size_t>& source) {
  return IBWT(source.first, source.second);
}

}  // namespace BurrowsWheelerTransform
}  // namespace ResearchLibrary

#endif  // INCLUDES_BURROWS_WHEELER_TRANSFORM_H_
