// Copyright 2015 pixie.grasper
/// \file range-coder.h
/// \brief Implementation of the RangeCoder
/// \author pixie.grasper

#ifndef INCLUDES_UNIVERSAL_CODING_H_
#define INCLUDES_UNIVERSAL_CODING_H_

#include <cmath>
#include <vector>
#include <utility>

#ifdef RESEARCHLIB_OFFLINE_TEST
#include "./size-type.h"
#include "./bit-byte-converter.h"
#else
#include <size-type.h>
#include <bit-byte-converter.h>
#endif

/// \namespace ResearchLibrary
/// \namespace ResearchLibrary::UniversalCoding
namespace ResearchLibrary {
namespace UniversalCoding {

/// \fn UnaryCodingEncode(const std::vector<T>& data)
/// \brief Unary Coding Encode Function
/// \param[in] data non-negative sequence
/// \return \c std::pair of encoded sequence and
///         \c std::pair of length of the original sequence and sample
template <typename T>
auto UnaryCodingEncode(const std::vector<T>& data) {
  BitsToBytes<8> buffer{};
  for (std::size_t i = 0; i < data.size(); i++) {
    for (T value{}; value < data[i]; value++) {
      buffer.put(0, 1);
    }
    buffer.put(1, 1);
  }
  return std::make_pair(buffer.seek_to_byte_boundary(),
         std::make_pair(data.size(), T{}));
}

/// \fn UnaryCodingDecode(const std::vector<uint8_t>& data, std::size_t length)
/// \brief Unary Coding Decode Function
/// \param[in] data sequence
/// \param[in] length length of the original sequence
/// \return original sequence as \c std::vector<T>
template <typename T>
auto UnaryCodingDecode(const std::vector<uint8_t>& data, std::size_t length) {
  std::vector<T> ret(length);
  BytesToBits<8> buffer(data);
  for (std::size_t i = 0; i < length; i++) {
    T value{};
    for (; buffer.get(1) != 1; value++) {}
    ret[i] = std::move(value);
  }
  return ret;
}

/// \fn UnaryCodingDecode(const std::pair<std::vector<uint8_t>,
///                             std::pair<std::size_t, T>>& pair)
/// \brief Unary Coding Decode Function
/// \param[in] pair \c std::pair of data sequence and
///                 \c std::pair of length of the original sequence and
///                                 sample
/// \return original sequence as \c std::vector<T>
template <typename T>
auto UnaryCodingDecode(const std::pair<std::vector<uint8_t>,
                             std::pair<std::size_t, T>>& tuple) {
  return UnaryCodingDecode<T>(tuple.first, tuple.second.first);
}

/// \fn GammaCodingEncode(const std::vector<T>& data)
/// \brief Gamma Coding Encode Function
/// \param[in] data positive sequence
/// \return \c std::pair of encoded sequence and
///         \c std::pair of length of the original sequence and sample
template <typename T>
auto GammaCodingEncode(const std::vector<T>& data) {
  BitsToBytes<8> buffer{};
  for (std::size_t i = 0; i < data.size(); i++) {
    auto width = static_cast<unsigned_integer_t>(std::floor(std::log2(data[i])) + 1);
    buffer.put(0, width - 1);
    for (unsigned_integer_t j = 1; j <= width; j++) {
      buffer.put(static_cast<size_type_t<8>>(data[i]) >> (width - j), 1);
    }
  }
  return std::make_pair(buffer.seek_to_byte_boundary(),
         std::make_pair(data.size(), T{}));
}

/// \fn GammaCodingDecode(const std::vector<uint8_t>& data, std::size_t length)
/// \brief Gamma Coding Decode Function
/// \param[in] data sequence
/// \param[in] length length of the original sequence
/// \return decoded sequence
template <typename T>
auto GammaCodingDecode(const std::vector<uint8_t>& data, std::size_t length) {
  std::vector<T> ret(length);
  BytesToBits<8> buffer(data);
  for (std::size_t i = 0; i < length; i++) {
    unsigned_integer_t width = 0;
    for (; buffer.get(1) != 1; width++) {}
    T value = 1;
    for (; width != 0; width--) {
      value = (value << 1) + T(buffer.get(1));
    }
    ret[i] = value;
  }
  return ret;
}

/// \fn GammaCodingDecode(const std::pair<std::vector<uint8_t>,
///                             std::pair<std::size_t, T>>& tuple)
/// \brief Gamma Coding Decode Function
/// \param[in] tuple \c std::pair of data sequence as \c std::vector<uint8_t>
///            and \c std::pair of length of the original sequence and sample
/// \return decoded sequence
template <typename T>
auto GammaCodingDecode(const std::pair<std::vector<uint8_t>,
                             std::pair<std::size_t, T>>& tuple) {
  return GammaCodingDecode<T>(tuple.first, tuple.second.first);
}

/// \fn DeltaCodingEncode(const std::vector<T>& data)
/// \brief Delta Coding Encode Function
/// \param[in] data positive sequence
/// \return \c std::pair of encoded sequence and \c std::pair of length of the
///         original sequence and sample
template <typename T>
auto DeltaCodingEncode(const std::vector<T>& data) {
  BitsToBytes<8> buffer{};
  for (std::size_t i = 0; i < data.size(); i++) {
    auto width =
      static_cast<unsigned_integer_t>(std::floor(std::log2(data[i])) + 1);
    auto width_of_width =
      static_cast<unsigned_integer_t>(std::floor(std::log2(width)) + 1);
    buffer.put(0, width_of_width - 1);
    for (unsigned_integer_t j = 1; j <= width_of_width; j++) {
      buffer.put(width >> (width_of_width - j), 1);
    }
    for (unsigned_integer_t j = 2; j <= width; j++) {
      buffer.put(static_cast<unsigned_integer_t>(data[i]) >> (width - j), 1);
    }
  }
  return std::make_pair(buffer.seek_to_byte_boundary(),
         std::make_pair(data.size(), T{}));
}

/// \fn DeltaCodingDecode(const std::vector<uint8_t>& data, std::size_t length)
/// \brief Delta Coding Decode Function
/// \param[in] data sequencee
/// \param[in] length length of the original sequence
/// \return decoded sequence
template <typename T>
auto DeltaCodingDecode(const std::vector<uint8_t>& data, std::size_t length) {
  std::vector<T> ret(length);
  BytesToBits<8> buffer(data);
  for (std::size_t i = 0; i < length; i++) {
    unsigned_integer_t width_of_width = 0, width = 1;
    for (; buffer.get(1) != 1; width_of_width++) {}
    for (; width_of_width != 0; width_of_width--) {
      width = (width << 1) + buffer.get(1);
    }
    T value = 1;
    for (; width > 1; width--) {
      value = (value << 1) + T(buffer.get(1));
    }
    ret[i] = value;
  }
  return ret;
}

/// \fn DeltaCodingDecode(const std::pair<std::vector<uint8_t>,
///                              std::pair<std::size_t, T>>& tuple)
/// \brief Delta Coding Decode Function
/// \param[in] tuple \c std::pair of data sequence as \c std::vector<uint8_t>
///            and \c std::pair of length of the original sequence and sample
/// \return decoded sequence
template <typename T>
auto DeltaCodingDecode(const std::pair<std::vector<uint8_t>,
                              std::pair<std::size_t, T>>& tuple) {
  return DeltaCodingDecode<T>(tuple.first, tuple.second.first);
}

/// \fn OmegaCodingEncode(const std::vector<T>& data)
/// \brief Omega Coding Encode Function
/// \param[in] data positive sequence
/// \return \c std::pair of encoded sequence and \c std::pair of length of the
///         original sequence and sample
template <typename T>
auto OmegaCodingEncode(const std::vector<T>& data) {
  BitsToBytes<8> buffer{};
  for (std::size_t i = 0; i < data.size(); i++) {
    std::vector<T> buffer2{0};
    auto n = data[i];
    while (n != 1) {
      buffer2.push_back(n);
      n = static_cast<T>(std::floor(std::log2(n)));
    }
    for (auto it = buffer2.rbegin();; ++it) {
      if (*it == 0) {
        buffer.put(0, 1);
        break;
      }
      auto width = static_cast<unsigned_integer_t>(std::floor(std::log2(*it)) + 1);
      for (unsigned_integer_t j = 1; j <= width; j++) {
        buffer.put(static_cast<size_type_t<8>>(*it >> (width - j)), 1);
      }
    }
  }
  return std::make_pair(buffer.seek_to_byte_boundary(),
         std::make_pair(data.size(), T{}));
}

/// \fn OmegaCodingDecode(const std::vector<uint8_t>& data, std::size_t length)
/// \brief Omega Coding Decode Function
/// \param[in] data sequence
/// \param[in] length length of the original sequence
/// \return decoded sequence
template <typename T>
auto OmegaCodingDecode(const std::vector<uint8_t>& data, std::size_t length) {
  std::vector<T> ret(length);
  BytesToBits<8> buffer(data);
  for (std::size_t i = 0; i < length; i++) {
    size_type_t<8> n = 1;
    while (buffer.get(1) != 0) {
      auto width = n;
      n = 1;
      for (std::size_t j = 0; j < width; j++) {
        n = (n << 1) + buffer.get(1);
      }
    }
    ret[i] = T(n);
  }
  return ret;
}

/// \fn OmegaCodingDecode(const std::pair<std::vector<uint8_t>,
///                              std::pair<std::size_t, T>>& tuple)
/// \brief Omega Coding Decode Function
/// \param[in] tuple \c std::pair of data sequence as \c std::vector<uint8_t>
///            and \c std::pair of length of the original sequence and sample
/// \return decoded sequence
template <typename T>
auto OmegaCodingDecode(const std::pair<std::vector<uint8_t>,
                              std::pair<std::size_t, T>>& tuple) {
  return OmegaCodingDecode<T>(tuple.first, tuple.second.first);
}

/// \fn GolombCodingEncode(const std::vector<T>& data, T m)
/// \brief Golomb Coding Encode Function
/// \param[in] data non-negative sequence
/// \param[in] m positive integer
/// \return \c std::pair of encoded sequence and \c std::pair of length of the
///         original sequence and sample
template <typename T>
auto GolombCodingEncode(const std::vector<T>& data, T m) {
  BitsToBytes<8> buffer{};
  auto b = T(std::ceil(std::log2(m)));
  if ((m & (m - 1)) == 0) {
    for (std::size_t i = 0; i < data.size(); i++) {
      auto d = data[i];
      auto q = d / m;
      auto r = d % m;
      for (T j = 0; j < q; j++) {
        buffer.put(0, 1);
      }
      buffer.put(1, 1);
      for (T j = 1; j <= b; j++) {
        buffer.put(static_cast<size_type_t<8>>(r >> (b - j)), 1);
      }
    }
  } else {
    auto bb = (T(1) << b) - m;
    auto c = b - 1;
    for (std::size_t i = 0; i < data.size(); i++) {
      auto d = data[i];
      auto q = d / m;
      auto r = d % m;
      for (T j = 0; j < q; j++) {
        buffer.put(0, 1);
      }
      buffer.put(1, 1);
      if (r < bb) {
        for (T j = 1; j <= c; j++) {
          buffer.put(static_cast<size_type_t<8>>(r >> (c - j)), 1);
        }
      } else {
        r += (T(1) << b) - m;
        for (T j = 1; j <= b; j++) {
          buffer.put(static_cast<size_type_t<8>>(r >> (b - j)), 1);
        }
      }
    }
  }
  return std::make_pair(buffer.seek_to_byte_boundary(),
         std::make_pair(data.size(), T{}));
}

/// \fn GolombCodingDecode(const std::vector<uint8_t>& data,
///                         const T& m,
///                         std::size_t length)
/// \brief Golomb Coding Decode Function
/// \param[in] data sequence
/// \param[in] m positive integer
/// \param[in] length length of the original sequence
/// \return decoded sequence
template <typename T>
auto GolombCodingDecode(const std::vector<uint8_t>& data,
                         const T& m,
                         std::size_t length) {
  std::vector<T> ret(length);
  BytesToBits<8> buffer(data);
  auto b = T(std::ceil(std::log2(m)));
  if ((m & (m - 1)) == 0) {
    for (std::size_t i = 0; i < length; i++) {
      T q{}, r{};
      for (; buffer.get(1) != 1; q++) {}
      for (T j = 1; j <= b; j++) {
        r = (r << 1) + T(buffer.get(1));
      }
      ret[i] = q * m + r;
    }
  } else {
    auto bb = (T(1) << b) - m;
    auto c = b - 1;
    for (std::size_t i = 0; i < length; i++) {
      T q{}, r{};
      for (; buffer.get(1) != 1; q++) {}
      for (T j = 1; j <= c; j++) {
        r = (r << 1) + T(buffer.get(1));
      }
      if (r >= bb) {
        r = (r << 1) + T(buffer.get(1));
        r -= (T(1) << b) - m;
      }
      ret[i] = q * m + r;
    }
  }
  return ret;
}

/// \fn GolombCodingDecode(const std::pair<std::vector<uint8_t>,
///                               std::pair<std::size_t, T>>& tuple,
///                         const T& m)
/// \brief Golomb Coding Decode Function
/// \param[in] tuple \c std::pair of sequence and \c std::pair of length
///            of the original sequence and sample
/// \param[in] m positive integer
/// \return decoded sequence
template <typename T>
auto GolombCodingDecode(const std::pair<std::vector<uint8_t>,
                               std::pair<std::size_t, T>>& tuple,
                         const T& m) {
  return GolombCodingDecode<T>(tuple.first, m, tuple.second.first);
}

}  // namespace UniversalCoding
}  // namespace ResearchLibrary

#endif  // INCLUDES_UNIVERSAL_CODING_H_
