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
struct Word {
  bool matched;
  T character;
  std::size_t first, length;
};

template <typename T>
struct ExplicitState {
  // word
  std::size_t k, p, depth;
  // suffix link
  std::weak_ptr<ExplicitState<T>> f;
  // transitions
  std::map<T, std::shared_ptr<ExplicitState<T>>> g;
};

template <typename T>
class SuffixTree {
 private:
  std::shared_ptr<ExplicitState<T>> root_state;
  std::vector<std::pair<std::size_t, std::size_t>> matched;
  static constexpr std::size_t infty = size_type<sizeof(std::size_t)>::max / 2;

  void print(const std::vector<T>& data,
             std::size_t index,
             std::shared_ptr<ExplicitState<T>> state,
             int color = 31) {
    if (state == nullptr) {
      fprintf(stderr, "(nil)");
      return;
    }
    // branching-states have at least two transitions
    if (!state->g.empty()) {
      fprintf(stderr, "[1;%dm[[0m ", color);
      for (auto i = state->k; i <= state->p && i <= data.size(); i++) {
        fprintf(stderr, "%d ", data[i - 1]);
      }
      fprintf(stderr, "| ");
      for (auto it = state->g.begin(); it != state->g.end();
           ++it) {
        print(data, index, it->second, color + 1);
        fprintf(stderr, " ");
      }
      fprintf(stderr, "[1;%dm][0m", color);
    } else {  // leaves have not transitions
      fprintf(stderr, "[1;%dm([0m ", color);
      for (auto i = state->k; i <= index + 1; i++) {
        fprintf(stderr, "%d ", data[i - 1]);
      }
      fprintf(stderr, "[1;%dm)[0m", color);
    }
    return;
  }

  auto create_state() {
    auto ret = std::make_shared<ExplicitState<T>>();
    ret->k = 1;
    ret->p = 0;
    ret->depth = 0;
    return ret;
  }

  auto create_state(std::size_t k, std::size_t p, std::size_t depth) {
    auto ret = std::make_shared<ExplicitState<T>>();
    ret->k = k;
    ret->p = p;
    ret->depth = depth;
    return ret;
  }

  auto create_root_state() {
    return create_state(1, 0, 0);
  }

  auto create_perp_state() {
    return nullptr;
  }

  auto get_t_i(std::size_t i, const std::vector<T>& data) {
    return data[i - 1];
  }

  auto create_new_transition(std::shared_ptr<ExplicitState<T>> state,
                             std::size_t k,
                             std::size_t p,
                             const std::vector<T>& data) {
    if (state == nullptr) {
      fprintf(stderr, "cannot create new transition because null.\n");
      fprintf(stderr, "create_new_transition.2.\n");
      return;
    }
    auto t_k = get_t_i(k, data);
    if (state->g.find(t_k) != state->g.end()) {
      fprintf(stderr, "cannot create new transition because already exists.\n");
      fprintf(stderr, "create_new_transition.1.\n");
      return;
    }
    state->g[t_k] = create_state(k, p, state->depth + state->p - state->k + 1);
    return;
  }

  auto has_a_transition(std::shared_ptr<ExplicitState<T>> state,
                        const T& a) {
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
      create_new_transition(r, i, infty, data);
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
                      const T& t,
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
        auto t_k = get_t_i(k, data);
        auto t_kpk1 = get_t_i(k_prime + p - k + 1, data);
        s->g[t_k] = r;
        r->k = k_prime;
        r->p = k_prime + p - k;
        r->depth = s->depth + s->p - s->k + 1;
        r->g[t_kpk1] = s_prime;
        s_prime->k = k_prime + p - k + 1;
        s_prime->p = p_prime;
        s_prime->depth = r->depth + r->p - r->k + 1;
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
      // while (ssize_t(p_prime - k_prime) <= ssize_t(p - k)) {
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

  auto find(const std::vector<T>& data,
            std::size_t left_index,
            std::size_t right_index) {
    auto explicit_state = root_state;
    std::size_t edge = 0, length = 0;
    while (left_index <= right_index) {
      auto ch = get_t_i(left_index, data);
      if (!explicit_state->g.empty() &&
          explicit_state->p - explicit_state->k + 1 <= length) {
        if (explicit_state->g.find(ch) == explicit_state->g.end()) {
          return false;
        } else {
          length -= explicit_state->p - explicit_state->k + 1;
          explicit_state = explicit_state->g[ch];
          edge = explicit_state->k - 1;
        }
      } else {
        if (data[edge + length] == ch) {
          left_index++;
          length++;
        } else {
          return false;
        }
      }
    }
    return true;
  }

 public:
  // construction of STree(T) for string T = t_1 t_2 ... #
  // in alphabet Sigma = {t_-1, ..., t_-m}; # is the end marker
  // not appearing elsewhere in T.
  SuffixTree(const std::vector<T>& data, std::size_t minimum_length)
      : root_state(nullptr),
        matched(data.size()) {
    // create state root and perp;
    root_state = create_root_state();
    // create suffix link f'(root) = perp;
    root_state->f = std::shared_ptr<ExplicitState<T>>(nullptr);
    // s <- root;
    auto s = root_state;
    // k <- 1;
    std::size_t k = 1;
    for (std::size_t i = 1; i <= data.size(); i++) {
      auto pair_1 = update(s, k, i, data);
      s = pair_1.first;
      k = pair_1.second;
      auto pair_2 = canonize(s, k, i, data);
      s = pair_2.first;
      k = pair_2.second;
      for (std::size_t j = 1; j <= i; j++) {
        auto found = find(data, j, i);
        if (!found) {
          fprintf(stderr, "error - %zu:%zu = ", j, i);
          for (auto h = j; h <= i; h++) {
            fprintf(stderr, "%d ", data[h]);
          }
          fprintf(stderr, "\n");
        }
      }
    }
#if 0
    for (std::size_t i = 0; i < data.size(); i++) {
      fprintf(stderr, "%zu %zu %d %zu\n",
          matched[i].first, matched[i].second, data[i], i);
    }
#endif
    return;
  }

  auto& get() {
    return matched;
  }
};

template <typename T>
auto Encode(const std::vector<T>& data, std::size_t minimum_length) {
  SuffixTree<T> tree(data, minimum_length);
  std::vector<Word<T>> ret{};
  return ret;
}

}  // namespace ZivLempel77
}  // namespace ResearchLibrary

#endif  // INCLUDES_ZIV_LEMPEL_77_H_
