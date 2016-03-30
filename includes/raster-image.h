// Copyright 2015 pixie.grasper
/// \file raster-image.h
/// \brief Useful raster image library
/// \author pixie.grasper

#ifndef INCLUDES_RASTER_IMAGE_H_
#define INCLUDES_RASTER_IMAGE_H_

/// \privatesection
int gets();
/// \publicsection
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <type_traits>

#ifdef RESEARCHLIB_OFFLINE_TEST
#include "./size-type.h"
#include "./file-vector-converter.h"
#include "./deflate.h"
#include "./check-sum.h"
#else
#include <size-type.h>
#include <file-vector-converter.h>
#include <deflate.h>
#include <check-sum.h>
#endif

/// \namespace ResearchLibrary
/// \namespace ResearchLibrary::Graphics
namespace ResearchLibrary {
namespace Graphics {

/// \privatesection
struct Color {
  size_type_t<2> red, green, blue, alpha;

  Color() = default;
};

template <std::size_t N, typename std::enable_if<N == 1>::type* = nullptr>
auto readle(const std::vector<std::uint8_t>& buffer, std::size_t index) {
  return buffer[index];
}

template <std::size_t N, typename std::enable_if<N != 1>::type* = nullptr>
auto readle(const std::vector<std::uint8_t>& buffer, std::size_t index) {
  return size_type_t<N>(readle<N / 2>(buffer, index)
         + (size_type_t<N>(readle<N / 2>(buffer, index + N / 2)) << N * 4));
}

template <std::size_t N, typename std::enable_if<N == 1>::type* = nullptr>
auto writele(std::vector<std::uint8_t>&& buffer,
             std::size_t index,
             size_type_t<N> word) {
  buffer[index] = word;
  return std::move(buffer);
}

template <std::size_t N, typename std::enable_if<N != 1>::type* = nullptr>
auto writele(std::vector<std::uint8_t>&& buffer,
             std::size_t index,
             size_type_t<N> word) {
  buffer = writele<N / 2>(std::move(buffer), index, size_type_t<N / 2>(word));
  buffer = writele<N / 2>(std::move(buffer), index + N / 2, word >> N * 4);
  return std::move(buffer);
}

template <std::size_t N, typename std::enable_if<N == 1>::type* = nullptr>
auto writebe(std::vector<std::uint8_t>&& buffer,
             std::size_t index,
             size_type_t<N> word) {
  buffer[index] = word;
  return std::move(buffer);
}

template <std::size_t N, typename std::enable_if<N != 1>::type* = nullptr>
auto writebe(std::vector<std::uint8_t>&& buffer,
             std::size_t index,
             size_type_t<N> word) {
  buffer = writebe<N / 2>(std::move(buffer), index + N / 2,
                          size_type_t<N / 2>(word));
  buffer = writebe<N / 2>(std::move(buffer), index, word >> N * 4);
  return std::move(buffer);
}

class RasterImage {
 private:
  std::size_t width_, height_, depth_;
  std::unordered_map<std::size_t,
                     std::unordered_map<std::size_t, Color>> pixmap;

 public:
  RasterImage() = default;

  RasterImage(std::size_t width, std::size_t height, std::size_t depth = 0)
    : width_(width),
      height_(height),
      depth_(depth),
      pixmap{} {
    return;
  }

  void resize(std::size_t width, std::size_t height) {
    width_ = width;
    height_ = height;
    return;
  }

  std::size_t width() const {
    return width_;
  }

  std::size_t height() const {
    return height_;
  }

  std::size_t depth() const {
    return depth_;
  }

  std::size_t& depth() {
    return depth_;
  }

  const Color pixel(std::size_t x, std::size_t y) const {
    if (x < width_ && y < height_) {
      auto it = pixmap.find(x);
      if (it != pixmap.end()) {
        auto it2 = it->second.find(y);
        if (it2 != it->second.end()) {
          return it2->second;
        }
      }
    }
    return Color();
  }

  Color& pixel(std::size_t x, std::size_t y) {
    return pixmap[x][y];
  }

  void save_as_bitmap(const char* file_name) {
    std::vector<std::uint8_t> buffer{};
    buffer.resize(14 + 40);
    buffer[0] = 'B';
    buffer[1] = 'M';
    buffer = writele<4>(std::move(buffer), 10, 54);
    buffer = writele<4>(std::move(buffer), 14, 40);
    buffer = writele<4>(std::move(buffer), 18, size_type_t<4>(width()));
    buffer = writele<4>(std::move(buffer), 22, size_type_t<4>(height()));
    buffer = writele<2>(std::move(buffer), 26, 1);
    buffer = writele<2>(std::move(buffer), 28, 24);
    buffer = writele<4>(std::move(buffer), 34,
        size_type_t<4>(width() * height() * depth() / 8));
    buffer = writele<4>(std::move(buffer), 38, 3780);
    buffer = writele<4>(std::move(buffer), 42, 3780);
    buffer.resize(buffer.size() + width() * height() * 3);
    std::size_t diff = 0;
    for (std::size_t Y = 0; Y < height(); Y++) {
      auto y = height() - Y - 1;
      for (std::size_t x = 0; x < width(); x++) {
        auto index = 54 + (Y * width() + diff + x) * depth() / 8;
        buffer[index] = static_cast<std::uint8_t>(pixel(x, y).blue >> 8);
        buffer[index + 1] = static_cast<std::uint8_t>(pixel(x, y).green >> 8);
        buffer[index + 2] = static_cast<std::uint8_t>(pixel(x, y).red >> 8);
      }
      if (width() * depth() / 8 % 4 != 0) {
        diff += 4 - width() * depth() / 8 % 4;
      }
    }
    auto size = buffer.size();
    buffer = writele<4>(std::move(buffer), 2, size_type_t<4>(size));
    File::save_as(file_name, buffer);
    return;
  }

  std::uint8_t paeth_predictor(std::uint8_t a, std::uint8_t b, std::uint8_t c) {
    auto p = a + b - c;
    auto pa = std::abs(p - a);
    auto pb = std::abs(p - b);
    auto pc = std::abs(p - c);
    if (pa <= pb && pa <= pc) {
      return a;
    } else if (pb <= pc) {
      return b;
    } else {
      return c;
    }
  }

  std::vector<std::uint8_t> filter(std::uint8_t type,
                                   std::size_t bpp,
                                   const std::vector<std::uint8_t>& raw,
                                   const std::vector<std::uint8_t>& prior) {
    std::vector<std::uint8_t> ret(raw.size());
    switch (type) {
      case 0:
        for (std::size_t i = 0; i < ret.size(); i++) {
          ret[i] = raw[i];
        }
        break;
      case 1:
        for (std::size_t i = 0; i < bpp; i++) {
          ret[i] = raw[i];
        }
        for (auto i = bpp; i < ret.size(); i++) {
          ret[i] = raw[i] - raw[i - bpp];
        }
        break;
      case 2:
        for (std::size_t i = 0; i < ret.size(); i++) {
          ret[i] = raw[i] - prior[i];
        }
        break;
      case 3:
        for (std::size_t i = 0; i < bpp; i++) {
          ret[i] = raw[i] - prior[i] / 2;
        }
        for (auto i = bpp; i < ret.size(); i++) {
          ret[i] = raw[i] - (ret[i - bpp] + prior[i]) / 2;
        }
        break;
      case 4:
        for (std::size_t i = 0; i < bpp; i++) {
          ret[i] = raw[i] - paeth_predictor(0, prior[i], 0);
        }
        for (auto i = bpp; i < ret.size(); i++) {
          ret[i] = raw[i] - paeth_predictor
            (raw[i - bpp], prior[i], prior[i - bpp]);
        }
        break;
    }
    return ret;
  }

  void save_as_png16a(const char* file_name) {
    std::vector<std::uint8_t> buffer = {137, 80, 78, 71, 13, 10, 26, 10};
    std::vector<std::uint8_t> IHDR(8 + 17), IHDR_data(17);
    IHDR = writebe<4>(std::move(IHDR), 0, 13);
    IHDR_data[0] = 'I';
    IHDR_data[1] = 'H';
    IHDR_data[2] = 'D';
    IHDR_data[3] = 'R';
    IHDR_data = writebe<4>(std::move(IHDR_data), 4, size_type_t<4>(width()));
    IHDR_data = writebe<4>(std::move(IHDR_data), 8, size_type_t<4>(height()));
    IHDR_data[12] = 16;  // Bit depth
    IHDR_data[13] = 6;  // Color type = R, G, B, Alpha
    IHDR_data[14] = 0;  // Compression method
    IHDR_data[15] = 0;  // Filter method
    IHDR_data[16] = 0;  // Interlace method
    for (std::size_t i = 0; i < IHDR_data.size(); i++) {
      IHDR[i + 4] = IHDR_data[i];
    }
    IHDR = writebe<4>(std::move(IHDR),
                      IHDR.size() - 4,
                      CheckSum::CRC32(IHDR_data, 0));
    buffer.insert(buffer.end(), IHDR.begin(), IHDR.end());
    std::vector<std::uint8_t> IDAT_data(4);
    IDAT_data[0] = 'I';
    IDAT_data[1] = 'D';
    IDAT_data[2] = 'A';
    IDAT_data[3] = 'T';
    std::vector<std::uint8_t> source_buffer(8 * width() * height() + height());
    for (std::size_t y = 0; y < height(); y++) {
      source_buffer[y * width() * 8 + y] = 0;
      for (std::size_t x = 0; x < width(); x++) {
        auto index = y * width() * 8 + x * 8 + y + 1;
        auto&& p = pixel(x, y);
        source_buffer = writebe<2>
          (std::move(source_buffer), index, p.red);
        source_buffer = writebe<2>
          (std::move(source_buffer), index + 2, p.green);
        source_buffer = writebe<2>
          (std::move(source_buffer), index + 4, p.blue);
        source_buffer = writebe<2>
          (std::move(source_buffer), index + 6, p.alpha);
      }
    }
    auto compressed_data = Deflate::Encode(source_buffer);
    std::vector<std::uint8_t> zlib_form(6 + compressed_data.size());
    zlib_form[0] = 0x78;
    zlib_form[1] = 0xda;
    for (std::size_t i = 0; i < compressed_data.size(); i++) {
      zlib_form[i + 2] = compressed_data[i];
    }
    zlib_form = writebe<4>(std::move(zlib_form),
                           zlib_form.size() - 4,
                           CheckSum::Adler32(source_buffer, 1));
    IDAT_data.insert(IDAT_data.end(), zlib_form.begin(), zlib_form.end());
    std::vector<std::uint8_t> IDAT(8 + IDAT_data.size());
    IDAT = writebe<4>(std::move(IDAT), 0, size_type_t<4>(IDAT_data.size() - 4));
    for (std::size_t i = 0; i < IDAT_data.size(); i++) {
      IDAT[i + 4] = IDAT_data[i];
    }
    IDAT = writebe<4>(std::move(IDAT),
                      IDAT.size() - 4,
                      CheckSum::CRC32(IDAT_data, 0));
    buffer.insert(buffer.end(), IDAT.begin(), IDAT.end());
    std::vector<std::uint8_t> IEND(8 + 4), IEND_data(4);
    IEND = writebe<4>(std::move(IEND), 0, 0);
    IEND_data[0] = 'I';
    IEND_data[1] = 'E';
    IEND_data[2] = 'N';
    IEND_data[3] = 'D';
    for (std::size_t i = 0; i < IEND_data.size(); i++) {
      IEND[i + 4] = IEND_data[i];
    }
    IEND = writebe<4>(std::move(IEND),
                      IEND.size() - 4,
                      CheckSum::CRC32(IEND_data, 0));
    buffer.insert(buffer.end(), IEND.begin(), IEND.end());
    File::save_as(file_name, buffer);
    return;
  }

  void save_as_png(const char* file_name) {
    save_as_png16a(file_name);
    return;
  }
};

template <typename = int>
auto load_bitmap(const std::vector<std::uint8_t>& buffer) {
  RasterImage ret{};
  std::size_t start_offset = readle<4>(buffer, 10);
  bool top_to_bottom;
  std::size_t info_header_size = readle<4>(buffer, 14);
  if (info_header_size == 12) {
    // OS/2 BMPCOREHEADER
    auto width = readle<2>(buffer, 18);
    auto height = readle<2>(buffer, 20);
    if (height & 0x8000) {
      height = -height;
      top_to_bottom = true;
    } else {
      top_to_bottom = false;
    }
    ret.resize(width, height);
    ret.depth() = readle<2>(buffer, 24);
  } else {
    // Windows BMPINFOHEADER
    auto width = readle<4>(buffer, 18);
    auto height = readle<4>(buffer, 22);
    if (height & 0x80000000) {
      height = -height;
      top_to_bottom = true;
    } else {
      top_to_bottom = false;
    }
    ret.resize(width, height);
    ret.depth() = readle<2>(buffer, 28);
  }

  if (ret.depth() == 24 || ret.depth() == 32) {
    std::size_t diff = 0;
    for (std::size_t Y = 0; Y < ret.height(); Y++) {
      auto y = Y;
      if (!top_to_bottom) {
        y = ret.height() - Y - 1;
      }
      for (std::size_t x = 0; x < ret.width(); x++) {
        auto index = start_offset
                   + (Y * ret.width() + diff + x) * ret.depth() / 8;
        auto&& pixel = ret.pixel(x, y);
        pixel.red = readle<1>(buffer, index + 2) * 0x0101;
        pixel.green = readle<1>(buffer, index + 1) * 0x0101;
        pixel.blue = readle<1>(buffer, index) * 0x0101;
        if (ret.depth() == 32) {
          pixel.alpha = readle<1>(buffer, index + 3);
        } else {
          pixel.alpha = size_type<sizeof(pixel.alpha)>::max;
        }
      }
      if (ret.width() * ret.depth() / 8 % 4 != 0) {
        diff += 4 - ret.width() * ret.depth() / 8 % 4;
      }
    }
  }
  return ret;
}

template <typename = int>
auto load(const char* file_name) {
  auto buffer = File::load(file_name);
  if (buffer[0] == 'B' && buffer[1] == 'M') {
    return load_bitmap(buffer);
  }
  return RasterImage{};
}

}  // namespace Graphics
}  // namespace ResearchLibrary

#endif  // INCLUDES_RASTER_IMAGE_H_
