// Copyright 2015 pixie.grasper
/// \file check-sum.h
/// \brief Implementation of the Check-Sum Functions
/// \author pixie.grasper

#ifndef INCLUDES_CHECK_SUM_H_
#define INCLUDES_CHECK_SUM_H_

#include <cstdint>
#include <vector>

#ifdef RESEARCHLIB_OFFLINE_TEST
#include "./size-type.h"
#else
#include <size-type.h>
#endif

/// \namespace ResearchLibrary
/// \namespace ResearchLibrary::CheckSum
namespace ResearchLibrary {
namespace CheckSum {

/// \privatesection
struct CRC32Table {
  std::uint32_t array[256];

  constexpr CRC32Table() : array{} {
    for (std::size_t i = 0; i < 256; i++) {
      auto c = size_type_t<4>(i);
      for (auto j = 0; j < 8; j++) {
        if ((c & 1) == 1) {
          c = 0xEDB88320 ^ (c >> 1);
        } else {
          c >>= 1;
        }
      }
      array[i] = c;
    }
    return;
  }

  constexpr std::uint32_t operator[](std::size_t i) const {
    return array[i];
  }
};

/// \publicsection
/// \fn CRC32(const std::vector<std::uint8_t>& data, const T& start)
/// \brief CRC-32 Check Sum Function
/// \param[in] data sequence
/// \param[in] start initial value of the calculation. Usually, equals to 0.
/// \return crc-32 check-sum
template <typename T>
auto CRC32(const std::vector<std::uint8_t>& data, const T& start) {
  constexpr CRC32Table table{};
  auto c = ~size_type_t<4>(start);
  for (std::size_t i = 0; i < data.size(); i++) {
    c = table[(c ^ data[i]) & 0xff] ^ (c >> 8);
  }
  return ~c;
}

/// \fn Alder32(const std::vector<std::uint8_t>& data, const T& start)
/// \brief Alder-32 Check Sum Function
/// \param[in] data sequence
/// \param[in] start initial value of the calculation. Usually, equals to 1.
/// \return alder-32 check-sum
template <typename T>
auto Alder32(const std::vector<std::uint8_t>& data, const T& start) {
  auto s1 = size_type_t<4>(start) & 0xffff;
  auto s2 = (size_type_t<4>(start) >> 16) & 0xffff;
  for (std::size_t i = 0; i < data.size(); i++) {
    s1 = (s1 + data[i]) % 65521;
    s2 = (s2 + s1) % 65521;
  }
  return (s2 << 16) + s1;
}

}  // namespace CheckSum
}  // namespace ResearchLibrary

#endif  // INCLUDES_CHECK_SUM_H_
