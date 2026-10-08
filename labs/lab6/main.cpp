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
public:
    explicit NumberCounter(std::string n_str, int modulo)
        : n_digits_(), length_(n_str.length()), m_(modulo) {
        
        n_digits_.reserve(length_);
        for (char ch : n_str) {
            n_digits_.push_back(ch - '0');
        }

        const std::size_t num_pairs = (length_ * (length_ + 1)) / 2;

        const std::size_t total_size = num_pairs * static_cast<std::size_t>(m_) * 3;
        dp_.assign(total_size, -1);
    }

    std::int64_t count() {
        return solve(0, 0, 0, false, LexState::Equal);
    }

private:
    std::vector<int> n_digits_;
    std::size_t length_;
    int m_;

    std::vector<std::int64_t> dp_;

    std::int64_t& dp_at(
        std::size_t pos, int rem, std::size_t len, LexState lex_state
    ) noexcept {
        const std::size_t pair_idx = (pos * (pos + 1)) / 2 + len;

        const auto lex_idx = static_cast<std::size_t>(lex_state);
        const std::size_t idx = (pair_idx * static_cast<std::size_t>(m_) + rem) * 3 + lex_idx;
        
        return dp_[idx];
    }

    static constexpr LexState next_lex_state(
        int d, int target_digit, LexState current_state
    ) noexcept {
        if (current_state != LexState::Equal) 
            return current_state;
        
        if (d < target_digit)  
            return LexState::Less;

        if (d == target_digit) 
            return LexState::Equal;

        return LexState::Greater;
    }

    std::int64_t solve(
        std::size_t pos,
        int rem,
        std::size_t len,
        bool is_less_num,
        LexState lex_state
    ) {
        if (pos == length_) {
            if (len == 0 || rem != 0 || !is_less_num)
                return 0;

            const bool is_strict_prefix = (lex_state == LexState::Equal && len < length_);
            return (lex_state == LexState::Less || is_strict_prefix) ? 1 : 0;
        }

        if (is_less_num) {
            if (const std::int64_t cached = dp_at(pos, rem, len, lex_state); cached != -1)
                return cached;
        }

        std::int64_t total_ways = 0;
        const int limit = is_less_num ? 9 : n_digits_[pos];

        for (int d = 0; d <= limit; ++d) {
            const int next_rem = (rem * 10 + d) % m_;
            const bool next_less_num = is_less_num || (d < n_digits_[pos]);

            std::size_t next_len = len;
            LexState computed_lex_state = lex_state;

            if (len == 0) {
                if (d > 0) {
                    next_len = 1;
                    computed_lex_state = next_lex_state(d, n_digits_[0], LexState::Equal);
                }
            } else {
                next_len = len + 1;
                computed_lex_state = next_lex_state(d, n_digits_[len], lex_state);
            }

            total_ways += solve(pos + 1, next_rem, next_len, next_less_num, computed_lex_state);
        }

        if (is_less_num)
            dp_at(pos, rem, len, lex_state) = total_ways;

        return total_ways;
    }
};

int main() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::string n_str;
    int m = 0;

    if (std::cin >> n_str >> m) {
        NumberCounter counter(std::move(n_str), m);
        std::cout << counter.count() << '\n';
    }

    return 0;
}