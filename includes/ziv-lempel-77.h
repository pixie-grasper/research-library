// Copyright 2015 pixie.grasper
/// \file ziv-lempel-77.h
/// \brief Implementation of the Ziv-Lempel 77
/// \author pixie.grasper

#ifndef INCLUDES_ZIV_LEMPEL_77_H_
#define INCLUDES_ZIV_LEMPEL_77_H_

/// \privatesection
int gets();
/// \publicsection
#include <vector>
#include <map>
#include <memory>
#include <utility>
#include <algorithm>
#include <cmath>

#ifdef RESEARCHLIB_OFFLINE_TEST
#include "./size-type.h"
#include "./bit-byte-converter.h"
#else
#include <size-type.h>
#include <bit-byte-converter.h>
#endif

/// \namespace ResearchLibrary
/// \namespace ResearchLibrary::ZivLempel77
namespace ResearchLibrary {
namespace ZivLempel77 {

/// \privatesection
template <typename T>
struct Character {
  T character;
  bool valid;

  Character() : character{}, valid(false) {
    return;
  }

  explicit Character(const T& ch) : character(ch), valid(true) {
    return;
  }

  bool operator==(const Character<T>& ch) const {
    if (valid && ch.valid) {
      return character == ch.character;
    } else {
      return false;
    }
  }

  bool operator<(const Character<T>& ch) const {
    if (valid && ch.valid) {
      return character < ch.character;
    } else if (valid && !ch.valid) {
      return true;
    } else {
      return false;
    }
  }
};

template <typename T>
struct ExplicitState {
  // word
  std::size_t k, p, depth;
  // suffix link
  std::weak_ptr<ExplicitState<T>> f;
  // transitions
  std::map<Character<T>, std::shared_ptr<ExplicitState<T>>> g;
  // parent
  std::weak_ptr<ExplicitState<T>> parent;
};

template <typename T>
class SuffixTree {
 private:
  std::shared_ptr<ExplicitState<T>> root_state;
  std::vector<std::pair<std::size_t, std::size_t>> matched;
  std::size_t window_width;
  static constexpr std::size_t infty = size_type<sizeof(std::size_t)>::max / 2;

  auto create_state() {
    auto ret = std::make_shared<ExplicitState<T>>();
    ret->k = 1;
    ret->p = 0;
    ret->depth = 0;
    return ret;
  }

  auto create_root_state() {
    return create_state();
  }

  auto get_t_i(std::size_t i, const std::vector<T>& data) {
    if (i <= data.size()) {
      return Character<T>(data[i - 1]);
    } else {
      return Character<T>();
    }
  }

  void update_range_to_root(std::shared_ptr<ExplicitState<T>> s,
                            std::size_t k) {
    while (s != nullptr && s->p < k) {
      s->k = k - (s->p - s->k + 1);
      s->p = k - 1;
      k = s->k;
      s = s->parent.lock();
    }
    return;
  }

  void connect(std::shared_ptr<ExplicitState<T>> s,
               std::size_t k,
               std::size_t p,
               std::shared_ptr<ExplicitState<T>> r,
               const std::vector<T>& data) {
    auto t_k = get_t_i(k, data);
    s->g[t_k] = r;
    r->k = k;
    r->p = p;
    r->depth = s->depth + s->p - s->k + 1;
    r->parent = s;
    update_range_to_root(s, k);
    return;
  }

  auto has_a_transition(std::shared_ptr<ExplicitState<T>> state,
                        const Character<T>& a) {
    if (state == nullptr) {
      // perp-state . a -> root-state where any a in the alphabet
      return true;
    } else {
      if (state->g.find(a) == state->g.end()) {
        return false;
      } else {
        return true;
      }
    }
  }

  std::pair<std::shared_ptr<ExplicitState<T>>,
            std::pair<std::size_t, std::size_t>>
  find_transition(std::shared_ptr<ExplicitState<T>> state,
                  std::size_t k,
                  const std::vector<T>& data) {
    if (state == nullptr) {
      // prep-state . a -> root-state where any a in the alphabet
      return std::make_pair(root_state, std::make_pair(-size_t(1), -size_t(1)));
    } else {
      auto t_k = get_t_i(k, data);
      return std::make_pair(state->g[t_k],
             std::make_pair(state->g[t_k]->k, state->g[t_k]->p));
    }
  }

  void update_matched(std::shared_ptr<ExplicitState<T>> s, std::size_t i) {
    if (s == nullptr) {
      return;
    }
    auto matched_to = s->k - s->depth - 1;
    auto matched_length = s->depth + s->p - s->k + 1;
    auto matched_from = i - matched_length - 1;
    while (matched_to + window_width < i) {
      s = s->parent.lock();
      if (s == nullptr) {
        return;
      }
      matched_to = s->k - s->depth - 1;
      matched_length = s->depth + s->p - s->k + 1;
    }
    if (matched[matched_from].first < matched_length) {
      matched[matched_from].first = matched_length;
      matched[matched_from].second = matched_to;
    }
    return;
  }

  auto update(std::shared_ptr<ExplicitState<T>> s,
              std::size_t k,
              std::size_t i,
              const std::vector<T>& data) {
    // (s, k, i - 1) is the canonical reference pair for the active point;
    // oldr <- root;
    auto old_r = root_state;
    // (end-point, r) <- test-and-split(s, k, i - 1, t_i);
    auto t_i = get_t_i(i, data);
    auto pair_1 = test_and_split(s, k, i - 1, t_i, data);
    auto end_point = pair_1.first;
    auto r = pair_1.second;
    // while not (end-point) do
    while (!end_point) {
      // creawte new transition g'(r, i, infty) = r' where r' is a new state;
      auto r_prime = create_state();
      update_matched(r, i);
      connect(r, i, infty, r_prime, data);
      // if oldr != root then create new suffix link f'(oldr) = r;
      if (old_r != root_state) {
        old_r->f = r;
      }
      // oldr <- r;
      old_r = r;
      // (s, k) <- canonize(f'(s), k, i - 1);
      auto pair_2 = canonize(s->f.lock(), k, i - 1, data);
      s = pair_2.first;
      k = pair_2.second;
      // (end-point, r) <- test-and-split(s, k, i - 1, t_i);
      auto pair_3 = test_and_split(s, k, i - 1, t_i, data);
      end_point = pair_3.first;
      r = pair_3.second;
    }
    // if oldr != root then create new suffix link f'(oldr) = s;
    if (old_r != root_state) {
      old_r->f = s;
    }
    // return (s, k);
    return std::make_pair(s, k);
  }

  auto test_and_split(std::shared_ptr<ExplicitState<T>> s,
                      std::size_t k,
                      std::size_t p,
                      const Character<T>& t,
                      const std::vector<T>& data) {
    // if k <= p then
    if (k <= p) {
      // let g'(s, k', p') = s' be the t_k-transition from s;
      auto pair = find_transition(s, k, data);
      auto s_prime = pair.first;
      auto k_prime = pair.second.first;
      auto p_prime = pair.second.second;
      // if t = t_(k'+p-k+1) then return (true, s)
      if (t == get_t_i(k_prime + p - k + 1, data)) {
        return std::make_pair(true, s);
      } else {  // else
        // replace the t_k-transition above by transitions
        // g'(s, k', k'+p-k) = r and g'(r, k'+p-k+1, p') = s'
        // where r is a new state;
        auto r = create_state();
        connect(s, k_prime, k_prime + p - k, r, data);
        connect(r, k_prime + p - k + 1, p_prime, s_prime, data);
        // return (false, r)
        return std::make_pair(false, r);
      }
    } else {  // else
      // if there is no t-transition from s then return (false, s)
      // else return (true, s)
      if (!has_a_transition(s, t)) {
        return std::make_pair(false, s);
      } else {
        return std::make_pair(true, s);
      }
    }
  }

  auto canonize(std::weak_ptr<ExplicitState<T>> ws,
                std::size_t k,
                std::size_t p,
                const std::vector<T>& data) {
    // if p < k then return (s, k)
    auto s = ws.lock();
    if (p < k) {
      return std::make_pair(s, k);
    } else {  // else
      // find the t_k-transition g'(s, k', p') = s' from s;
      auto pair = find_transition(s, k, data);
      auto s_prime = pair.first;
      auto k_prime = pair.second.first;
      auto p_prime = pair.second.second;
      // while p'-k' <= p - k do
      while (p_prime + k <= p + k_prime) {
        // k <- k + p' - k' + 1;
        k = k + p_prime - k_prime + 1;
        // s <- s';
        s = s_prime;
        // if k <= p then find the t_k-transition g'(s, k', p') = s' from s;
        if (k <= p) {
          pair = find_transition(s, k, data);
          s_prime = pair.first;
          k_prime = pair.second.first;
          p_prime = pair.second.second;
        }
      }
      // return (s, k).
      return std::make_pair(s, k);
    }
  }

 public:
  SuffixTree() = default;

  // construct suffix-tree with the Ukkonen's Algorithm
  // construction of STree(T) for string T = t_1 t_2 ... #
  // in alphabet Sigma = {t_-1, ..., t_-m}; # is the end marker
  // not appearing elsewhere in T.
  void build(const std::vector<T>& data, std::size_t ww) {
    matched.resize(data.size());
    window_width = ww;
    // create state root and perp;
    root_state = create_root_state();
    // create suffix link f'(root) = perp;
    root_state->f = std::shared_ptr<ExplicitState<T>>(nullptr);
    // s <- root;
    auto s = root_state;
    // k <- 1;
    std::size_t k = 1;
    for (std::size_t i = 1; i <= data.size() + 1; i++) {
      auto pair_1 = update(s, k, i, data);
      s = pair_1.first;
      k = pair_1.second;
      auto pair_2 = canonize(s, k, i, data);
      s = pair_2.first;
      k = pair_2.second;
    }
    return;
  }

  auto& get() {
    return matched;
  }
};

template <typename T>
struct Word {
  std::size_t position, start, length;
  T character;
};

template <typename T>
struct Work {
  unsigned_integer_t cost;
  std::size_t from, to;

  Work() : cost(size_type<unsigned_integer_size>::max),
           from(0),
           to(0) {
    return;
  }
};

/// \publicsection
/// \fn Encode(const std::vector<T>& data,
///            std::size_t minimum_length,
///            std::size_t window_width)
/// \brief Ziv-Lempel 77 Encode Function
/// \param[in] data sequence
/// \param[in] minimum_length supre border of the length
/// \param[in] window_width width of the window
/// \return \c std::pair of tuple as \c std::vector<Word<T>> and
///         length of the original sequence
template <typename T>
auto Encode(const std::vector<T>& data,
            std::size_t minimum_length,
            std::size_t window_width) {
  SuffixTree<T> tree{};
  tree.build(data, window_width);
  auto&& matched_length = tree.get();
  for (std::size_t i = 0; i < matched_length.size(); i++) {
    if (matched_length[i].first <= minimum_length) {
      matched_length[i].first = 0;
    }
  }
#if 1
  // DEFLATE's fixed Huffman code table
  auto unmatch_cost = [=](std::size_t i) -> unsigned_integer_t {
    auto t = data[i];
    if (t <= 143) {
      return 8;
    } else {
      return 9;
    }
  };
  auto match_cost = [](std::size_t l, std::size_t d) {
    auto length_cost = [](std::size_t length) -> unsigned_integer_t {
      if (length <= 10) {
        return 7;
      } else if (length <= 18) {
        return 8;
      } else if (length <= 34) {
        return 9;
      } else if (length <= 66) {
        return 10;
      } else if (length <= 114) {
        return 11;
      } else if (length <= 130) {
        return 12;
      } else if (length <= 257) {
        return 13;
      } else {
        return 8;
      }
    };
    auto distance_cost = [](std::size_t distance) -> unsigned_integer_t {
      if (distance <= 4) {
        return 5;
      } else {
        return 3 + unsigned_integer_t(std::ceil(std::log2(distance)));
      }
    };
    return length_cost(l) + distance_cost(d);
  };
#else
  auto unmatch_cost = [](std::size_t) -> unsigned_integer_t {
    return 1;
  };
  auto match_cost = [](std::size_t, std::size_t) -> unsigned_integer_t {
    return 1;
  };
#endif
  std::vector<Work<T>> work(data.size() + 1);
  work[0].cost = 0;
  for (std::size_t i = 0; i < data.size(); i++) {
    if (work[i].cost + unmatch_cost(i) < work[i + 1].cost) {
      work[i + 1].cost = work[i].cost + unmatch_cost(i);
      work[i + 1].from = i;
    }
    auto length = matched_length[i].first;
    auto distance = i - matched_length[i].second;
    auto cost = match_cost(length, distance);
    if (length != 0 &&
        i + length < work.size() &&
        work[i].cost + cost < work[i + length].cost) {
      work[i + length].cost = work[i].cost + cost;
      work[i + length].from = i;
    }
  }
  for (auto i = data.size(); i > 0;) {
    work[work[i].from].to = i;
    i = work[i].from;
  }
  std::vector<Word<T>> ret(data.size());
  std::size_t j = 0;
  for (std::size_t i = 0; i < data.size();) {
    Word<T> word{};
    word.position = i;
    word.start = matched_length[i].second;
    word.length = work[i].to - i - 1;
    if (word.length == 0) {
      word.start = 0;
    }
    word.character = data[work[i].to - 1];
    ret[j] = word;
    j++;
    i = work[i].to;
  }
  ret.resize(j);
  return std::make_pair(ret, data.size());
}

/// \fn Decode(const std::vector<Word<T>>& data, std::size_t length)
/// \brief Ziv-Lempel 77 Decode Function
/// \param[in] data tuple sequence
/// \param[in] length length of the original sequence
/// \return decoded sequence
template <typename T>
auto Decode(const std::vector<Word<T>>& data, std::size_t length) {
  std::vector<T> ret(length);
  std::size_t k = 0;
  for (std::size_t i = 0; i < data.size(); i++) {
    for (std::size_t j = 0; j < data[i].length; j++) {
      ret[k] = ret[data[i].start + j];
      k++;
    }
    ret[k] = data[i].character;
    k++;
  }
  return ret;
}

/// \fn Decode(const std::pair<std::vector<Word<T>>, std::size_t>& pair)
/// \brief Ziv-Lempel 77 Decode Function
/// \param[in] pair \c std::pair of tuple sequence and
///            length of the original sequence
/// \return decoded sequence
template <typename T>
auto Decode(const std::pair<std::vector<Word<T>>, std::size_t>& pair) {
  return Decode(pair.first, pair.second);
}

}  // namespace ZivLempel77
}  // namespace ResearchLibrary

#endif  // INCLUDES_ZIV_LEMPEL_77_H_
