// Copyright 2015 pixie.grasper
/// \file file.h
/// \brief Useful File reader/writer.
/// \author pixie.grasper

#ifndef INCLUDES_FILE_H_
#define INCLUDES_FILE_H_

#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

/// \privatesection
int gets();
/// \publicsection
#include <cstdint>
#include <vector>

#ifdef RESEARCHLIB_OFFLINE_TEST
#include "./size-type.h"
#else
#include <size-type.h>
#endif

/// \namespace ResearchLibrary
namespace ResearchLibrary {

/// \class File
/// \brief File to vector<std::uint8_t> class
class File {
 private:
  const char* file_name;
  std::vector<std::uint8_t> buffer;

 public:
  File() = default;

  /// \fn File(const File&)
  /// \brief default copy constructor
  File(const File&) = default;

  /// \fn File(File&&)
  /// \brief default move constructor
  File(File&&) = default;

  /// \fn File(const char* name)
  /// \brief constructor
  /// \param[in] name name of the file
  explicit File(const char* name) : file_name(name), buffer{} {
    load();
    return;
  }

  /// \fn File(std::vector<uint8_t>&& contents)
  /// \brief constructor
  /// \param[in] contents buffer contains sequence
  explicit File(std::vector<uint8_t>&& contents)
    : file_name(nullptr), buffer(std::move(contents)) {
    return;
  }

  /// \fn set_name(const char* name)
  /// \brief set the file name
  /// \param[in] name file name
  void set_name(const char* name) {
    file_name = name;
    return;
  }

  /// \fn get()
  /// \brief get the buffer
  /// \return \c std::vector<std::uint8_t>
  std::vector<std::uint8_t>& get() {
    return buffer;
  }

  /// \fn load(void)
  /// \brief load from the file
  /// \return \c std::vector<std::uint8_t>
  std::vector<std::uint8_t>& load() {
    buffer.resize(0);
    auto fd = open(file_name, O_RDONLY);
    if (fd == -1) {
      return buffer;
    }
    struct stat s;
    if (fstat(fd, &s) == -1) {
      return buffer;
    }
    buffer.resize(std::size_t(s.st_size));
    read(fd, &buffer[0], std::size_t(s.st_size));
    close(fd);
    return buffer;
  }

  /// \fn load(const char* file_name)
  /// \brief load from the file
  /// \param[in] file_name name of the file
  /// \return \c std::vector<std::uint8_t>
  static std::vector<std::uint8_t> load(const char* file_name) {
    File file(file_name);
    return file.get();
  }

  /// \fn save(void)
  /// \brief save to the file
  void save() const {
    if (file_name == nullptr) {
      return;
    }
    auto fd = open(file_name,
                   O_CREAT | O_TRUNC | O_WRONLY,
                   S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd == -1) {
      return;
    }
    write(fd, &buffer[0], buffer.size());
    close(fd);
    return;
  }

  /// \fn save_as(const char* name)
  /// \brief save to file as another name.
  /// \param[in] name file name
  void save_as(const char* name) {
    set_name(name);
    save();
    return;
  }

  /// \fn save_as(const char* name, const std::vector<std::uint8_t>& buffer)
  /// \brief save to file
  /// \param[in] name file name
  /// \param[in] buffer buffer contains to save
  static void save_as(const char* name,
                      const std::vector<std::uint8_t>& buffer) {
    auto fd = open(name,
                   O_CREAT | O_TRUNC | O_WRONLY,
                   S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd == -1) {
      return;
    }
    write(fd, &buffer[0], buffer.size());
    close(fd);
    return;
  }
};

}  // namespace ResearchLibrary

#endif  // INCLUDES_FILE_H_
