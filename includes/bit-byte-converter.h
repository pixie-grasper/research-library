// Copyright 2015 pixie.grasper
/// \file bit-byte-converter.h
/// \brief Useful bit-stream maps to byte-stream, and reverse it.
/// \author pixie.grasper

#ifndef INCLUDES_BIT_BYTE_CONVERTER_H_
#define INCLUDES_BIT_BYTE_CONVERTER_H_

/// \privatesection
int gets();
/// \publicsection
#include <vector>
#include <map>
#include <utility>

#ifdef RESEARCHLIB_OFFLINE_TEST
#include "./size-type.h"
#else
#include <size-type.h>
#endif

/// \namespace ResearchLibrary
namespace ResearchLibrary {

/// \class BitsToBytes
/// \brief converts bit stream to byte stream
template <size_t N>
class BitsToBytes {
 private:
  std::vector<uint8_t> data;
  size_t buffered_length;
  uint8_t buffered_bits;

  size_type_t<N> mask(size_t n) {
    return (size_type_t<N>(1) << n) - 1;
  }

 public:
  BitsToBytes() = default;

  /// \fn put(size_type_t<N> value, size_t length)
  /// \brief put a bits to the stream
  /// \param[in] value contains bit-stream
  /// \param[in] length length of the bit-stream
  void put(size_type_t<N> value, size_t length) {
    value &= mask(length);
    if (buffered_length + length >= 8) {
      // | <-   value  -> | <- buffered -> |
      // | <- rest -> | <-      8       -> |
      data.push_back(static_cast<uint8_t>(
            (buffered_bits | (value << buffered_length)) & 0xff));
      value >>= 8 - buffered_length;
      buffered_length = length + buffered_length - 8;
      while (buffered_length >= 8) {
        data.push_back(static_cast<uint8_t>(value & 0xff));
        value >>= 8;
        buffered_length -= 8;
      }
      buffered_bits = static_cast<uint8_t>(value & mask(buffered_length));
    } else {
      buffered_bits |= value << buffered_length;
      buffered_length += length;
    }
    return;
  }

  /// \fn rput(size_type_t<N> value, size_t length)
  /// \brief put a reversed bits to the stream
  /// \param[in] value contains bit-stream
  /// \param[in] length length of the bit-stream
  void rput(size_type_t<N> value, size_t length) {
    for (size_t i = 1; i <= length; i++) {
      put(value >> (length - i), 1);
    }
    return;
  }

  /// \fn seek_to_byte_boundary()
  /// \brief seek to byte-boundary with fill-zero
  /// \return returns current buffer as \c std::vector<uint8_t>
  std::vector<uint8_t>& seek_to_byte_boundary() {
    if (buffered_length != 0) {
      data.push_back(buffered_bits);
      buffered_bits = 0;
      buffered_length = 0;
    }
    return data;
  }
};

/// \class BytesToBits
/// \brief converts byte stream to bit stream
template <size_t N>
class BytesToBits {
 private:
  std::vector<uint8_t> buffer;
  size_t data_index, buffered_length;
  size_type_t<N> buffered_bits;

  size_type_t<N> mask(size_t n) {
    return (size_type_t<N>(1) << n) - 1;
  }

 public:
  /// \fn BytesToBits(const std::vector<uint8_t>& data)
  /// \brief Constructor of class BytesToBits
  /// \param[in] data buffer that contains bit-stream
  explicit BytesToBits(const std::vector<uint8_t>& data)
    : buffer(data), data_index(0), buffered_length(0), buffered_bits(0) {
    return;
  }

  /// \fn BytesToBits(std::vector<uint8_t>&& data)
  /// \brief Constructor of class BytesToBits
  /// \param[in] data buffer that contains bit-stream
  explicit BytesToBits(std::vector<uint8_t>&& data)
    : buffer(std::move(data)),
      data_index(0),
      buffered_length(0),
      buffered_bits(0) {
    return;
  }

  /// \fn get(size_t length)
  /// \brief get a value from the stream
  size_type_t<N> get(size_t length) {
    if (length > buffered_length) {
      // | <- buffer -> | <- buffered bits -> |
      // | <- rest -> | <-      value      -> |
      auto value = buffered_bits;
      auto stored_length = buffered_length;
      length -= buffered_length;
      buffered_bits = 0;
      buffered_length = 0;
      while (length > buffered_length) {
        buffered_bits |= size_type_t<N>(buffer[data_index]) << buffered_length;
        buffered_length += 8;
        data_index++;
      }
      value |= (buffered_bits & mask(length)) << stored_length;
      buffered_bits >>= length;
      buffered_length -= length;
      buffered_bits &= mask(buffered_length);
      return value;
    } else {
      // | <- 0 -> | <-   buffered bits    -> |
      // |         | <- rest -> | <- value -> |
      auto value = buffered_bits & mask(length);
      buffered_length -= length;
      buffered_bits >>= length;
      return value;
    }
  }

  /// \fn fetch(size_t length)
  /// \brief fetch a value from the stream
  size_type_t<N> fetch(size_t length) const {
    if (length > buffered_length) {
      auto value = buffered_bits;
      auto stored_length = buffered_length;
      length -= buffered_length;
      auto index = data_index;
      size_t extra_read_length = 0;
      size_type_t<N> extra_buffer = 0;
      while (length > extra_read_length) {
        extra_buffer |= size_type_t<N>(buffer[index]) << extra_read_length;
        extra_read_length += 8;
        index++;
      }
      return value | (extra_buffer & mask(length)) << stored_length;
    } else {
      return buffered_bits & mask(length);
    }
  }

  /// \fn rget(size_t length)
  /// \brief get a reversed value from the stream
  size_type_t<N> rget(size_t length) {
    size_type_t<N> ret = 0;
    for (size_t i = 0; i < length; i++) {
      ret = (ret << 1) | get(1);
    }
    return ret;
  }

  /// \fn rget(size_t length, size_type_t<N> upper_bits)
  /// \brief get a reversed value from the stream
  size_type_t<N> rget(size_t length, size_type_t<N> upper_bits) {
    for (size_t i = 0; i < length; i++) {
      upper_bits = (upper_bits << 1) | get(1);
    }
    return upper_bits;
  }

  /// \fn seek_to_byte_boundary()
  /// \brief seek to the byte-boundary
  void seek_to_byte_boundary() {
    if (buffered_length != 0) {
      buffered_length = 0;
      buffered_bits = 0;
      data_index++;
    }
    return;
  }
};

}  // namespace ResearchLibrary

#endif  // INCLUDES_BIT_BYTE_CONVERTER_H_
