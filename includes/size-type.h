// Copyright 2015 pixie.grasper
/// \file size-type.h
/// \brief Useful constexpred integer maps to type of integer.
/// \author pixie.grasper

#ifndef INCLUDES_SIZE_TYPE_H_
#define INCLUDES_SIZE_TYPE_H_

#include <cstdint>
#include <cstddef>

/// \namespace ResearchLibrary
namespace ResearchLibrary {

/// \struct size_type
/// \brief \c size_type template requires constexpr-ish integer
///        such as \c 1, \c 2, \c 4, \c sizeof(int), and so on.
///        if the integer equals to \c 1, \c 2, \c 4 or \c 8,
///        \c size_type::type will declared
///        and \c sizeof(size_type<N>::type) == \c N.
template <size_t N>
struct size_type;

/// \privatesection
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

/// \publicsection
/// \typedef size_type_t
/// \brief \c size_type_t template declaration
template <size_t N>
using size_type_t = typename size_type<N>::type;

/// \var unsigned_integer_size
/// \brief equals to \c sizeof(unsigned_integer_t).
constexpr auto unsigned_integer_size = sizeof(ptrdiff_t);

/// \c unsigned_integer_t declaration.
using unsigned_integer_t = size_type_t<unsigned_integer_size>;

}  // namespace ResearchLibrary

#endif  // INCLUDES_SIZE_TYPE_H_
