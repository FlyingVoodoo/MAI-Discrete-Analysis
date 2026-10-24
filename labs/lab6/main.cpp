#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <utility>

enum class LexState : std::uint8_t {
    Greater = 0,
    Equal   = 1,
    Less    = 2
};

class NumberCounter {
    using u64 = std::int64_t;
    using usize = std::size_t;

    struct Transition {
        usize next_len;
        LexState next_lex;
    };

    std::vector<int> n_digits_;
    usize length_;
    int m_;
    std::vector<u64> dp_;

    constexpr usize pair_index(usize pos, usize len) const noexcept {
        return (pos * (pos + 1)) / 2 + len;
    }

    u64 dp_at(usize pos, int rem, usize len, LexState lex_state) const noexcept {
        const usize pair_idx = pair_index(pos, len);
        const usize lex_idx = static_cast<usize>(lex_state);

        return dp_[(pair_idx * static_cast<usize>(m_) + rem) * 3 + lex_idx];
    }

    u64& dp_at_mut(usize pos, int rem, usize len, LexState lex_state) noexcept {
        const usize pair_idx = pair_index(pos, len);
        const usize lex_idx = static_cast<usize>(lex_state);

        return dp_[(pair_idx * static_cast<usize>(m_) + rem) * 3 + lex_idx];
    }

    static constexpr LexState next_lex_state(int d, int target, LexState state) noexcept {
        if (state != LexState::Equal) 
            return state;
        if (d < target)  
            return LexState::Less;
        if (d == target) 
            return LexState::Equal;

        return LexState::Greater;
    }

    Transition transit(int d, usize len, LexState state) const noexcept {
        if (len == 0) {
            return (d > 0) ? Transition{1, next_lex_state(d, n_digits_[0], LexState::Equal)}
                           : Transition{0, state};
        }
        return {len + 1, next_lex_state(d, n_digits_[len], state)};
    }

    u64 check_terminal(usize len, int rem, LexState lex, bool is_less_num) const noexcept {
        if (len == 0 || rem != 0 || !is_less_num) 
            return 0;
        const bool is_strict_prefix = (lex == LexState::Equal && len < length_);
        
        return (lex == LexState::Less || is_strict_prefix) ? 1 : 0;
    }

public:
    explicit NumberCounter(std::string_view n_str, int modulo)
        : length_(n_str.length()), m_(modulo) {
        
        n_digits_.reserve(length_);
        for (char ch : n_str) {
            n_digits_.push_back(ch - '0');
        }

        const usize num_pairs = pair_index(length_ - 1, length_ - 1) + 1;
        dp_.assign(num_pairs * static_cast<usize>(m_) * 3, -1);
    }

    u64 count() {
        for (int pos_signed = static_cast<int>(length_) - 1; pos_signed >= 0; --pos_signed) {
            const usize pos = static_cast<usize>(pos_signed);

            for (usize len = 0; len <= pos; ++len) {
                for (int rem = 0; rem < m_; ++rem) {
                    for (int lex_i = 0; lex_i < 3; ++lex_i) {
                        const LexState lex_state = static_cast<LexState>(lex_i);
                        u64 total_ways = 0;

                        for (int d = 0; d <= 9; ++d) {
                            const int next_rem = (rem * 10 + d) % m_;
                            const auto [next_len, computed_lex] = transit(d, len, lex_state);

                            if (pos + 1 == length_) {
                                total_ways += check_terminal(next_len, next_rem, computed_lex, true);
                            } else {
                                total_ways += dp_at(pos + 1, next_rem, next_len, computed_lex);
                            }
                        }

                        dp_at_mut(pos, rem, len, lex_state) = total_ways;
                    }
                }
            }
        }

        u64 root_ways = 0;
        
        struct State {
            usize pos;
            int rem;
            usize len;
            LexState lex;
            bool is_less;
        };

        std::vector<State> current_states;
        current_states.push_back({0, 0, 0, LexState::Equal, false});

        for (usize pos = 0; pos < length_; ++pos) {
            std::vector<State> next_states;
            const int limit = n_digits_[pos];

            for (const auto& st : current_states) {
                const int max_d = st.is_less ? 9 : limit;
                for (int d = 0; d <= max_d; ++d) {
                    const int next_rem = (st.rem * 10 + d) % m_;
                    const bool next_less = st.is_less || (d < limit);
                    const auto [next_len, computed_lex] = transit(d, st.len, st.lex);

                    if (pos + 1 == length_) {
                        root_ways += check_terminal(next_len, next_rem, computed_lex, next_less);
                    } else {
                        if (next_less) {
                            root_ways += dp_at(pos + 1, next_rem, next_len, computed_lex);
                        } else {
                            next_states.push_back({pos + 1, next_rem, next_len, computed_lex, next_less});
                        }
                    }
                }
            }
            current_states = std::move(next_states);
            if (current_states.empty()) break;
        }

        return root_ways;
    }
};

int main() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::string n_str;
    int m = 0;

    if (std::cin >> n_str >> m) {
        NumberCounter counter(n_str, m);
        std::cout << counter.count() << '\n';
    }

    return 0;
}