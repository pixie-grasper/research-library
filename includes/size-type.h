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
template <std::size_t N>
struct size_type;

/// \privatesection
template <>
struct size_type<1> {
  using type = std::uint8_t;
  static const type max = UINT8_MAX;
};

template <>
struct size_type<2> {
  using type = std::uint16_t;
  static const type max = UINT16_MAX;
};

template <>
struct size_type<4> {
  using type = std::uint32_t;
  static const type max = UINT32_MAX;
};

template <>
struct size_type<8> {
  using type = std::uint64_t;
  static const type max = UINT64_MAX;
};

/// \publicsection
/// \typedef size_type_t
/// \brief \c size_type_t template declaration
template <std::size_t N>
using size_type_t = typename size_type<N>::type;

/// \var unsigned_integer_size
/// \brief equals to \c sizeof(unsigned_integer_t).
constexpr auto unsigned_integer_size = sizeof(std::ptrdiff_t);

/// \c unsigned_integer_t declaration.
using unsigned_integer_t = size_type_t<unsigned_integer_size>;

}  // namespace ResearchLibrary

#endif  // INCLUDES_SIZE_TYPE_H_
