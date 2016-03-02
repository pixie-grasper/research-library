// Copyright 2015 pixie.grasper

#ifndef INCLUDES_SIZE_TYPE_H_
#define INCLUDES_SIZE_TYPE_H_

#include <stdint.h>
#include <cstddef>

namespace ResearchLibrary {

template <size_t N>
struct size_type {};

template <>
struct size_type<1> {
  using type = uint8_t;
};

template <>
struct size_type<2> {
  using type = uint16_t;
};

template <>
struct size_type<4> {
  using type = uint32_t;
};

template <>
struct size_type<8> {
  using type = uint64_t;
};

template <size_t N>
using size_type_t = typename size_type<N>::type;

constexpr auto unsigned_integer_size = sizeof(ptrdiff_t);
using unsigned_integer_t = size_type_t<unsigned_integer_size>;

}  // namespace ResearchLibrary

#endif  // INCLUDES_SIZE_TYPE_H_
