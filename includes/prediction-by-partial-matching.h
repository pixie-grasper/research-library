// Copyright 2015 pixie.grasper
/// \file prediction-by-partial-matching.h
/// \brief Implementation of the RangeCoder
/// \author pixie.grasper

#ifndef INCLUDES_PREDICTION_BY_PARTIAL_MATCHING_H_
#define INCLUDES_PREDICTION_BY_PARTIAL_MATCHING_H_

#include <stdint.h>

/// \privatesection
int gets();
/// \publicsection
#include <vector>
#include <map>
#include <set>
#include <list>
#include <utility>

#ifdef RESEARCHLIB_OFFLINE_TEST
#include "./size-type.h"
#include "./range-coder.h"
#else
#include <size-type.h>
#include <range-coder.h>
#endif

/// \enum Method
/// \brief Method{A,B,C,D} are defined.
enum Method {
  MethodA, MethodB, MethodC, MethodD,
};

/// \namespace ResearchLibrary
/// \namespace ResearchLibrary::PredictionByPartialMatching
namespace ResearchLibrary {
namespace PredictionByPartialMatching {

/// \class Predictor
/// \brief Predictor<typename T, int Depth, enum Method>
///        for the PPM Algorithm
template <typename T, int Depth, enum Method M>
class Predictor;

/// \class Predictor<T, 0, MethodA>
/// \brief Predictor that depth = 0, Method A for the PPM Algorithm
template <typename T>
class Predictor<T, 0, MethodA> {
 private:
  std::map<T, unsigned_integer_t> freq;
  const std::set<T> A;
  size_t n;
  bool escaping;
  // | <- detected -> | <- not detected -> |
  // | <-    n     -> | <-      1       -> |
  // | <-            n + 1              -> |

 public:
  /// \fn Predictor(const std::vector<T>& A)
  /// \brief Constructor of the predictor.
  explicit Predictor(const std::set<T>& A_)
      : freq{}, A(A_), n(0), escaping(false) {
    return;
  }

  /// \fn enter_escape_mode()
  /// \brief set escape-flag
  void enter_escape_mode() {
    escaping = true;
    return;
  }

  /// \fn leave_escape_mode()
  /// \brief unset escape-flag
  void leave_escape_mode() {
    escaping = false;
    return;
  }

  /// \fn has_to_escape(T value)
  /// \brief returns true if has to escape
  bool has_to_escape(T value) const {
    if (escaping) {
      return false;
    } else {
      if (freq.find(value) == freq.end()) {
        return true;
      } else {
        return false;
      }
    }
  }

  /// \fn denominator()
  /// \brief returns denominator on this context
  unsigned_integer_t denominator() const {
    if (escaping) {
      return A.size() - freq.size();
    } else {
      return n + 1;
    }
  }

  /// \fn numerator()
  /// \brief returns numerator on this context
  unsigned_integer_t numerator() const {
    if (escaping) {
      return A.size() - freq.size();
    } else {
      return n;
    }
  }

  /// \fn PDF(T value)
  /// \brief returns PDF numerator
  unsigned_integer_t PDF(T value) const {
    if (escaping) {
      return 1;
    } else {
      auto it = freq.find(value);
      if (it == freq.end()) {
        return 0;
      } else {
        return it->second;
      }
    }
  }

  /// \fn CDF(T value)
  /// \brief returns CDF numerator
  unsigned_integer_t CDF(T value) const {
    if (escaping) {
      unsigned_integer_t sum = 0;
      for (auto it = A.begin(); it != A.end(); ++it) {
        if (*it < value && freq.find(*it) == freq.end()) {
          sum++;
        }
      }
      return sum;
    } else {
      unsigned_integer_t sum = 0;
      for (auto it = freq.begin(); it != freq.end(); ++it) {
        if (it->first < value) {
          sum += it->second;
        } else {
          break;
        }
      }
      return sum;
    }
  }


  /// \fn ICDF(unsigned_integer_t cum)
  /// \brief returns Inverse CDF
  T ICDF(unsigned_integer_t cum) const {
    if (escaping) {
      unsigned_integer_t sum = 0;
      for (auto it = A.begin(); it != A.end(); ++it) {
        if (freq.find(*it) == freq.end()) {
          sum++;
          if (sum > cum) {
            return *it;
          }
        }
      }
    } else {
      unsigned_integer_t sum = 0;
      for (auto it = freq.begin(); it != freq.end(); ++it) {
        sum += it->second;
        if (sum > cum) {
          return it->first;
        }
      }
    }
    return T();
  }

  /// \fn update_frequency(T value)
  /// \brief update frequency on this context
  void update_frequency(T value) {
    if (freq.find(value) == freq.end()) {
      freq[value] = 1;
    } else {
      freq[value]++;
    }
    n++;
    return;
  }

  /// \fn update_sequence(T value)
  /// \brief update predecessor_list
  void update_sequence(T) {
    return;
  }

  /// \fn update_predictor(T value)
  /// \brief update this predictor
  void update_predictor(T value) {
    update_frequency(value);
    update_sequence(value);
    return;
  }
};

/// \class Predictor<T, Depth, MethodA>
/// \brief Predictor that depth > 0, Method A for the PPM Algorithm
template <typename T, int Depth>
class Predictor<T, Depth, MethodA> {
 private:
  Predictor<T, Depth - 1, MethodA> predictor;
  std::map<std::list<T>, std::map<T, unsigned_integer_t>> freq;
  std::list<T> predecessor_list;
  const std::set<T> A;
  std::map<std::list<T>, size_t> n;
  bool escaping;

 public:
  /// \fn Predictor(const std::vector<T>& A)
  /// \brief Constructor of the predictor.
  explicit Predictor(const std::set<T>& A_)
      : predictor(A_),
        freq{},
        predecessor_list{},
        A(A_),
        n{},
        escaping(false) {
    return;
  }

  /// \fn enter_escape_mode()
  /// \brief set escape-flag
  void enter_escape_mode() {
    if (escaping) {
      predictor.enter_escape_mode();
    } else {
      escaping = true;
    }
    return;
  }

  /// \fn leave_escape_mode()
  /// \brief unset escape-flag
  void leave_escape_mode() {
    escaping = false;
    predictor.leave_escape_mode();
    return;
  }

  /// \fn has_to_escape(T value)
  /// \brief returns true if has to escape
  bool has_to_escape(T value) const {
    if (escaping) {
      return predictor.has_to_escape(value);
    } else {
      if (predecessor_list.size() < Depth) {
        return true;
      }
      auto it = freq.find(predecessor_list);
      if (it != freq.end()) {
        auto it2 = it->second.find(value);
        if (it2 != it->second.end()) {
          return false;
        }
      }
      return true;
    }
  }

  /// \fn denominator()
  /// \brief returns denominator on this context
  unsigned_integer_t denominator() const {
    if (escaping) {
      return predictor.denominator();
    } else {
      if (predecessor_list.size() < Depth) {
        return 1;
      }
      auto it = n.find(predecessor_list);
      if (it != n.end()) {
        return it->second + 1;
      } else {
        return 1;
      }
    }
  }

  /// \fn numerator()
  /// \brief returns numerator on this context
  unsigned_integer_t numerator() const {
    if (escaping) {
      return predictor.numerator();
    } else {
      if (predecessor_list.size() < Depth) {
        return 0;
      }
      auto it = n.find(predecessor_list);
      if (it != n.end()) {
        return it->second;
      } else {
        return 0;
      }
    }
  }

  /// \fn PDF(T value)
  /// \brief returns PDF numerator
  unsigned_integer_t PDF(T value) const {
    if (escaping) {
      return predictor.PDF(value);
    } else {
      if (predecessor_list.size() < Depth) {
        return 0;
      }
      auto it = freq.find(predecessor_list);
      if (it != freq.end()) {
        auto it2 = it->second.find(value);
        if (it2 == it->second.end()) {
          return 0;
        } else {
          return it2->second;
        }
      }
      return 0;
    }
  }

  /// \fn CDF(T value)
  /// \brief returns CDF numerator
  unsigned_integer_t CDF(T value) const {
    if (escaping) {
      return predictor.CDF(value);
    } else {
      if (predecessor_list.size() < Depth) {
        return 0;
      }
      auto it = freq.find(predecessor_list);
      if (it != freq.end()) {
        unsigned_integer_t sum = 0;
        for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
          if (it2->first < value) {
            sum += it2->second;
          } else {
            break;
          }
        }
        return sum;
      }
      return 0;
    }
  }


  /// \fn ICDF(unsigned_integer_t cum)
  /// \brief returns Inverse CDF
  T ICDF(unsigned_integer_t cum) const {
    if (escaping) {
      return predictor.ICDF(cum);
    } else {
      if (predecessor_list.size() < Depth) {
        return T();
      }
      auto it = freq.find(predecessor_list);
      if (it != freq.end()) {
        unsigned_integer_t sum = 0;
        for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
          sum += it2->second;
          if (sum > cum) {
            return it2->first;
          }
        }
      }
    }
    return T();
  }

  /// \fn update_frequency(T value)
  /// \brief update frequency on this context
  void update_frequency(T value) {
    if (predecessor_list.size() == Depth) {
      if (freq[predecessor_list].find(value) == freq[predecessor_list].end()) {
        freq[predecessor_list][value] = 1;
      } else {
        freq[predecessor_list][value]++;
      }
      if (n.find(predecessor_list) == n.end()) {
        n[predecessor_list] = 1;
      } else {
        n[predecessor_list]++;
      }
    }
    if (escaping) {
      predictor.update_frequency(value);
    }
    predictor.update_sequence(value);
    return;
  }

  /// \fn update_sequence(T value)
  /// \brief update predecessor_list
  void update_sequence(T value) {
    if (predecessor_list.size() == Depth) {
      predecessor_list.pop_front();
    }
    predecessor_list.push_back(value);
    return;
  }

  /// \fn update_predictor(T value)
  /// \brief update this predictor
  void update_predictor(T value) {
    update_frequency(value);
    update_sequence(value);
    return;
  }
};

/// \class Predictor<T, 0, MethodB>
/// \brief Predictor that depth = 0, Method B for the PPM Algorithm
template <typename T>
class Predictor<T, 0, MethodB> {
 private:
  std::map<T, unsigned_integer_t> freq;
  std::set<T> once;
  const std::set<T> A;
  size_t n;
  bool escaping, escaping2;
  // | <- detected -> | <- not detected -> |
  // |   | <- once -> |                    |
  // |   | <-            u              -> |
  // | <-              n                -> |

 public:
  /// \fn Predictor(const std::vector<T>& A)
  /// \brief Constructor of the predictor.
  explicit Predictor(const std::set<T>& A_)
      : freq{}, once{}, A(A_), n(0), escaping(false), escaping2(false) {
    return;
  }

  /// \fn enter_escape_mode()
  /// \brief set escape-flag
  void enter_escape_mode() {
    if (escaping) {
      escaping2 = true;
    } else {
      escaping = true;
    }
    return;
  }

  /// \fn leave_escape_mode()
  /// \brief unset escape-flag
  void leave_escape_mode() {
    escaping = false;
    escaping2 = false;
    return;
  }

  /// \fn has_to_escape(T value)
  /// \brief returns true if has to escape
  bool has_to_escape(T value) const {
    if (escaping) {
      if (escaping2) {
        return false;
      } else {
        if (once.find(value) != once.end()) {
          return false;
        } else {
          return true;
        }
      }
    } else {
      if (freq.find(value) == freq.end()) {
        return true;
      } else if (once.find(value) != once.end()) {
        return true;
      } else {
        return false;
      }
    }
  }

  /// \fn denominator()
  /// \brief returns denominator on this context
  unsigned_integer_t denominator() const {
    if (escaping) {
      if (escaping2) {
        return A.size() - freq.size();
      } else {
        return A.size() - freq.size() + once.size();
      }
    } else {
      if (n == 0) {
        return 1;
      } else {
        return n;
      }
    }
  }

  /// \fn numerator()
  /// \brief returns numerator on this context
  unsigned_integer_t numerator() const {
    if (escaping) {
      if (escaping2) {
        return A.size() - freq.size();
      } else {
        return once.size();
      }
    } else {
      return n - freq.size();
    }
  }

  /// \fn PDF(T value)
  /// \brief returns PDF numerator
  unsigned_integer_t PDF(T value) const {
    if (escaping) {
      return 1;
    } else {
      auto it = freq.find(value);
      if (it == freq.end()) {
        return 0;
      } else {
        return it->second - 1;
      }
    }
  }

  /// \fn CDF(T value)
  /// \brief returns CDF numerator
  unsigned_integer_t CDF(T value) const {
    if (escaping) {
      if (escaping2) {
        unsigned_integer_t sum = 0;
        for (auto it = A.begin(); it != A.end(); ++it) {
          if (*it < value) {
            if (freq.find(*it) == freq.end()) {
              sum++;
            }
          } else {
            break;
          }
        }
        return sum;
      } else {
        unsigned_integer_t sum = 0;
        for (auto it = once.begin(); it != once.end(); ++it) {
          if (*it < value) {
            sum++;
          } else {
            break;
          }
        }
        return sum;
      }
    } else {
      unsigned_integer_t sum = 0;
      for (auto it = freq.begin(); it != freq.end(); ++it) {
        if (it->first < value) {
          sum += it->second - 1;
        } else {
          break;
        }
      }
      return sum;
    }
  }


  /// \fn ICDF(unsigned_integer_t cum)
  /// \brief returns Inverse CDF
  T ICDF(unsigned_integer_t cum) const {
    if (escaping) {
      if (escaping2) {
        unsigned_integer_t sum = 0;
        for (auto it = A.begin(); it != A.end(); ++it) {
          if (freq.find(*it) == freq.end()) {
            sum++;
            if (sum > cum) {
              return *it;
            }
          }
        }
      } else {
        unsigned_integer_t sum = 0;
        for (auto it = once.begin(); it != once.end(); ++it) {
          sum++;
          if (sum > cum) {
            return *it;
          }
        }
      }
    } else {
      unsigned_integer_t sum = 0;
      for (auto it = freq.begin(); it != freq.end(); ++it) {
        sum += it->second - 1;
        if (sum > cum) {
          return it->first;
        }
      }
    }
    return T();
  }

  /// \fn update_frequency(T value)
  /// \brief update frequency on this context
  void update_frequency(T value) {
    if (freq.find(value) == freq.end()) {
      freq[value] = 1;
      once.insert(value);
    } else {
      freq[value]++;
      once.erase(value);
    }
    n++;
    return;
  }

  /// \fn update_sequence(T value)
  /// \brief update predecessor_list
  void update_sequence(T) {
    return;
  }

  /// \fn update_predictor(T value)
  /// \brief update this predictor
  void update_predictor(T value) {
    update_frequency(value);
    update_sequence(value);
    return;
  }
};

/// \class Predictor<T, Depth, MethodB>
/// \brief Predictor that depth > 0, Method B for the PPM Algorithm
template <typename T, int Depth>
class Predictor<T, Depth, MethodB> {
 private:
  Predictor<T, Depth - 1, MethodB> predictor;
  std::map<std::list<T>, std::map<T, unsigned_integer_t>> freq;
  std::list<T> predecessor_list;
  const std::set<T> A;
  std::map<std::list<T>, size_t> n;
  bool escaping;

 public:
  /// \fn Predictor(const std::vector<T>& A)
  /// \brief Constructor of the predictor.
  explicit Predictor(const std::set<T>& A_)
      : predictor(A_),
        freq{},
        predecessor_list{},
        A(A_),
        n{},
        escaping(false) {
    return;
  }

  /// \fn enter_escape_mode()
  /// \brief set escape-flag
  void enter_escape_mode() {
    if (escaping) {
      predictor.enter_escape_mode();
    } else {
      escaping = true;
    }
    return;
  }

  /// \fn leave_escape_mode()
  /// \brief unset escape-flag
  void leave_escape_mode() {
    escaping = false;
    predictor.leave_escape_mode();
    return;
  }

  /// \fn has_to_escape(T value)
  /// \brief returns true if has to escape
  bool has_to_escape(T value) const {
    if (escaping) {
      return predictor.has_to_escape(value);
    } else {
      if (predecessor_list.size() < Depth) {
        return true;
      }
      auto it = freq.find(predecessor_list);
      if (it != freq.end()) {
        auto it2 = it->second.find(value);
        if (it2 != it->second.end()) {
          return false;
        }
      }
      return true;
    }
  }

  /// \fn denominator()
  /// \brief returns denominator on this context
  unsigned_integer_t denominator() const {
    if (escaping) {
      return predictor.denominator();
    } else {
      if (predecessor_list.size() < Depth) {
        return 1;
      }
      auto it = n.find(predecessor_list);
      if (it != n.end()) {
        return it->second + 1;
      } else {
        return 1;
      }
    }
  }

  /// \fn numerator()
  /// \brief returns numerator on this context
  unsigned_integer_t numerator() const {
    if (escaping) {
      return predictor.numerator();
    } else {
      if (predecessor_list.size() < Depth) {
        return 0;
      }
      auto it = n.find(predecessor_list);
      if (it != n.end()) {
        return it->second;
      } else {
        return 0;
      }
    }
  }

  /// \fn PDF(T value)
  /// \brief returns PDF numerator
  unsigned_integer_t PDF(T value) const {
    if (escaping) {
      return predictor.PDF(value);
    } else {
      if (predecessor_list.size() < Depth) {
        return 0;
      }
      auto it = freq.find(predecessor_list);
      if (it != freq.end()) {
        auto it2 = it->second.find(value);
        if (it2 == it->second.end()) {
          return 0;
        } else {
          return it2->second;
        }
      }
      return 0;
    }
  }

  /// \fn CDF(T value)
  /// \brief returns CDF numerator
  unsigned_integer_t CDF(T value) const {
    if (escaping) {
      return predictor.CDF(value);
    } else {
      if (predecessor_list.size() < Depth) {
        return 0;
      }
      auto it = freq.find(predecessor_list);
      if (it != freq.end()) {
        unsigned_integer_t sum = 0;
        for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
          if (it2->first < value) {
            sum += it2->second;
          } else {
            break;
          }
        }
        return sum;
      }
      return 0;
    }
  }


  /// \fn ICDF(unsigned_integer_t cum)
  /// \brief returns Inverse CDF
  T ICDF(unsigned_integer_t cum) const {
    if (escaping) {
      return predictor.ICDF(cum);
    } else {
      if (predecessor_list.size() < Depth) {
        return T();
      }
      auto it = freq.find(predecessor_list);
      if (it != freq.end()) {
        unsigned_integer_t sum = 0;
        for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
          sum += it2->second;
          if (sum > cum) {
            return it2->first;
          }
        }
      }
    }
    return T();
  }

  /// \fn update_frequency(T value)
  /// \brief update frequency on this context
  void update_frequency(T value) {
    if (predecessor_list.size() == Depth) {
      if (freq[predecessor_list].find(value) == freq[predecessor_list].end()) {
        freq[predecessor_list][value] = 1;
      } else {
        freq[predecessor_list][value]++;
      }
      if (n.find(predecessor_list) == n.end()) {
        n[predecessor_list] = 1;
      } else {
        n[predecessor_list]++;
      }
    }
    if (escaping) {
      predictor.update_frequency(value);
    }
    predictor.update_sequence(value);
    return;
  }

  /// \fn update_sequence(T value)
  /// \brief update predecessor_list
  void update_sequence(T value) {
    if (predecessor_list.size() == Depth) {
      predecessor_list.pop_front();
    }
    predecessor_list.push_back(value);
    return;
  }

  /// \fn update_predictor(T value)
  /// \brief update this predictor
  void update_predictor(T value) {
    update_frequency(value);
    update_sequence(value);
    return;
  }
};

/// \fn Encode(const std::vector<T>& data, const std::set<T>& A)
/// \brief PPM Algorithm Encode Function
/// \param[in] data sequence
/// \param[in] A data set
/// \return encoded sequence as \c std::vector<uint8_t>
template <typename T, int Depth, enum Method M>
auto Encode(const std::vector<T>& data, const std::set<T>& A) {
  Predictor<T, Depth, M> predictor(A);
  auto cont = RangeCoder::encode_init<unsigned_integer_size>();
  for (size_t i = 0; i < data.size(); i++) {
    while (predictor.has_to_escape(data[i])) {
      auto n = predictor.numerator();
      auto d = predictor.denominator();
      if (d != n) {
        cont = RangeCoder::encode_process(std::move(cont), n, d - n, d);
      }
      predictor.enter_escape_mode();
    }
    cont = RangeCoder::encode_process(std::move(cont),
                                      predictor.CDF(data[i]),
                                      predictor.PDF(data[i]),
                                      predictor.denominator());
    predictor.update_predictor(data[i]);
    predictor.leave_escape_mode();
  }
  return RangeCoder::encode_finish(std::move(cont));
}

/// \fn Encode(const std::vector<T>& data)
/// \brief PPM Algorithm Encode Function
/// \param[in] data sequence
/// \return \c std::pair of encoded sequence as \c std::vector<uint8_t> and
///         \c std::pair of data set and length of the original sequence
template <typename T, int Depth, enum Method M>
auto Encode(const std::vector<T>& data) {
  std::set<T> set{};
  for (size_t i = 0; i < data.size(); i++) {
    set.insert(data[i]);
  }
  return std::make_pair(Encode<T, Depth, M>(data, set),
         std::make_pair(set, data.size()));
}

/// \fn Decode(const std::vector<uint8_t>& data,
///            const std::set<T>& A,
///            size_t original_size)
/// \brief PPM Algorithm Decode Function
/// \param[in] data sequence
/// \param[in] A data set
/// \param[in] original_size length of the original sequence
/// \return data sequence as \c std::vector<T>
template <typename T, int Depth, enum Method M>
auto Decode(const std::vector<uint8_t>& data,
            const std::set<T>& A,
            size_t original_size) {
  Predictor<T, Depth, M> predictor(A);
  std::vector<T> ret{};
  auto cont = RangeCoder::decode_init<unsigned_integer_size, T>(data);
  for (size_t i = 0; i < original_size; i++) {
    while (RangeCoder::decode_split(cont,
                                    predictor.numerator(),
                                    predictor.denominator())) {
      auto n = predictor.numerator();
      auto d = predictor.denominator();
      if (d != n) {
        cont = RangeCoder::decode_process(std::move(cont), data, n, d - n, d);
      }
      predictor.enter_escape_mode();
    }
    auto p = RangeCoder::decode_fetch(cont, predictor.denominator());
    auto ch = predictor.ICDF(p);
    cont = RangeCoder::decode_process(std::move(cont),
                                      data,
                                      predictor.CDF(ch),
                                      predictor.PDF(ch),
                                      predictor.denominator());
    ret.push_back(ch);
    predictor.update_predictor(ch);
    predictor.leave_escape_mode();
  }
  return ret;
}

/// \fn Decode(const std::pair<std::vector<uint8_t>,
///                  std::pair<std::set<T>, size_t>>& tuple)
/// \brief PPM Algorithm Decode Function
/// \param[in] tuple \c std::pair of data sequence as \c std::vector<uint8_t>
///            and \c std::pair of data set as \c std::set<T> and
///            length of the original sequence
/// \return data sequence as \c std::vector<T>
template <typename T, int Depth, enum Method M>
auto Decode(const std::pair<std::vector<uint8_t>,
                  std::pair<std::set<T>, size_t>>& tuple) {
  return Decode<T, Depth, M>(tuple.first,
                             tuple.second.first,
                             tuple.second.second);
}

}  // namespace PredictionByPartialMatching
}  // namespace ResearchLibrary

#endif  // INCLUDES_PREDICTION_BY_PARTIAL_MATCHING_H_
