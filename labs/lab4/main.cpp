#include <iostream>
#include <cstdint>
#include <unordered_map>
#include <string>
#include <vector>
#include <iterator>
#include <algorithm>
#include <utility>
#include <cctype>
#include <ranges>
#include <concepts>

template <typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;

template <typename Pred, typename T>
concept Comparator = requires(const Pred& p, const T& a, const T& b) {
    { p(a, b) } -> std::convertible_to<bool>;
};

struct NumberIteratorEnd {};

template <Numeric T = uint32_t>
class NumberIterator {
private:
    const char* ptr_;
    T currentValue_;
    bool isEnd_;

    void advance() {
        while (ptr_ && *ptr_ && !std::isdigit(static_cast<unsigned char>(*ptr_))) {
            ++ptr_;
        }
        
        if (!ptr_ || !*ptr_ || !std::isdigit(static_cast<unsigned char>(*ptr_))) {
            isEnd_ = true;
            currentValue_ = 0;
            return;
        }
        
        currentValue_ = 0;
        while (ptr_ && *ptr_ && std::isdigit(static_cast<unsigned char>(*ptr_))) {
            currentValue_ = currentValue_ * 10 + (*ptr_ - '0');
            ++ptr_;
        }
    }

public:
    using iterator_concept = std::input_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = const T*;
    using reference = const T&;

    NumberIterator() : ptr_(nullptr), currentValue_(0), isEnd_(true) {}
    
    explicit NumberIterator(const char* str) : ptr_(str), currentValue_(0), isEnd_(false) {
        if (!str || !*str) {
            isEnd_ = true;
        } else {
            advance();
        }
    }

    T operator*() const {
        return currentValue_;
    }

    NumberIterator& operator++() {
        advance();
        return *this;
    }

    NumberIterator operator++(int) {
        NumberIterator tmp = *this;
        ++(*this);
        return tmp;
    }

    bool operator==(const NumberIteratorEnd&) const {
        return isEnd_;
    }

    bool operator!=(const NumberIteratorEnd&) const {
        return !isEnd_;
    }
};

template <Numeric T = uint32_t>
std::vector<T> parseNumbers(const std::string& line) {
    std::vector<T> result;
    NumberIterator<T> it(line.c_str());
    NumberIteratorEnd end;
    
    while (it != end) {
        result.push_back(*it);
        ++it;
    }
    return result;
}

template <typename T>
class View {
private:
    const T* data_;
    size_t size_;

public:
    constexpr View() : data_(nullptr), size_(0) {}
    constexpr View(const T* data, size_t size) : data_(data), size_(size) {}
    View(const std::vector<T>& vec) : data_(vec.data()), size_(vec.size()) {}
    View(const T* first, const T* last)
        : data_(first), size_(static_cast<size_t>(last - first)) {}

    constexpr const T* data() const { return data_; }
    constexpr size_t size() const { return size_; }
    constexpr bool empty() const { return size_ == 0; }

    constexpr const T& operator[](size_t index) const { return data_[index]; }

    constexpr const T* begin() const { return data_; }
    constexpr const T* end() const { return data_ + size_; }
};

template <Numeric T = uint32_t, Comparator<T> Pred = std::equal_to<T>>
class BoyerMooreSearcher {
    using Diff = std::ptrdiff_t;

    std::vector<T> pattern_;
    std::unordered_map<T, Diff> last_;
    std::vector<Diff> good_suffix_;
    Pred pred_;

public:
    explicit BoyerMooreSearcher(const View<T>& pattern, const Pred& pred = Pred())
        : pattern_(pattern.begin(), pattern.end()), pred_(pred) {
        if (!pattern_.empty()) {
            preprocessBadChar();
            preprocessGoodSuffix();
        }
    }

    explicit BoyerMooreSearcher(const std::vector<T>& pattern, const Pred& pred = Pred())
        : pattern_(pattern), pred_(pred) {
        if (!pattern_.empty()) {
            preprocessBadChar();
            preprocessGoodSuffix();
        }
    }

    explicit BoyerMooreSearcher(std::vector<T>&& pattern, const Pred& pred = Pred())
        : pattern_(std::move(pattern)), pred_(pred) {
        if (!pattern_.empty()) {
            preprocessBadChar();
            preprocessGoodSuffix();
        }
    }

    template <std::ranges::random_access_range Range, typename Callback>
    void operator()(Range&& range, Callback&& onMatch) const {
        auto first = std::ranges::begin(range);
        auto last = std::ranges::end(range);
        Diff n = std::distance(first, last);
        Diff m = static_cast<Diff>(pattern_.size());
        
        if (m == 0 || n < m)
            return;

        Diff shift = 0;
        while (shift <= n - m) {
            Diff j = m - 1;

            while (j >= 0 && pred_(pattern_[j], *(first + shift + j)))
                --j;

            if (j < 0) {
                if (onMatch(first + shift))
                    return;
                shift += std::max<Diff>(1, good_suffix_[0]);
            } else {
                Diff bc = 1;
                auto it = last_.find(*(first + shift + j));
                if (it != last_.end())
                    bc = std::max<Diff>(1, j - it->second);
                else
                    bc = j + 1;
                
                Diff gs = good_suffix_[j];
                shift += std::max(bc, gs);
            }
        }
    }

private:
    void preprocessBadChar() {
        for (Diff i = 0; i < static_cast<Diff>(pattern_.size()); ++i) {
            last_[pattern_[i]] = i;
        }
    }

    void preprocessGoodSuffix() {
        Diff m = static_cast<Diff>(pattern_.size());
        good_suffix_.assign(m, m);

        std::vector<Diff> suff(m, 0);
        suff[m - 1] = m;
        Diff g = m - 1, f = m - 1;

        for (Diff i = m - 2; i >= 0; --i) {
            if (i > g && suff[i + m - 1 - f] < i - g)
                suff[i] = suff[i + m - 1 - f];
            else {
                if (i < g) f = i;
                g = std::min(g, i);
                while (g >= 0 && pred_(pattern_[g], pattern_[g + m - 1 - f]))
                    --g;
                suff[i] = f - g;
            }
        }

        Diff last_prefix_len = 0;
        for (Diff i = m - 1; i >= 0; --i) {
            if (suff[i] == i + 1) {
                last_prefix_len = i + 1;
            }
            good_suffix_[i] = m - last_prefix_len;
        }

        for (Diff i = 0; i <= m - 2; ++i) {
            if (suff[i] > 0)
                good_suffix_[m - 1 - suff[i]] = m - 1 - i;
        }

        for (Diff i = 0; i < m; ++i) {
            if (good_suffix_[i] <= 0)
                good_suffix_[i] = 1;
        }
    }
};


struct TokenPos {
    uint32_t line;
    uint32_t word;
};

int main() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::string patternLine;
    if (!std::getline(std::cin, patternLine)) {
        return 0;
    }
    
    auto pattern = parseNumbers<uint32_t>(patternLine);
    if (pattern.empty()) {
        return 0;
    }

    std::vector<uint32_t> textValues;
    std::vector<TokenPos> tokenPositions;

    std::string line;
    uint32_t currentLineIdx = 1;

    while (std::getline(std::cin, line)) {
        uint32_t currentWordIdx = 1;
        NumberIterator<uint32_t> it(line.c_str());
        NumberIteratorEnd end;
        
        for (; it != end; ++it) {
            textValues.push_back(*it);
            tokenPositions.push_back({currentLineIdx, currentWordIdx});
            currentWordIdx++;
        }
        currentLineIdx++;
    }

    if (textValues.empty()) {
        return 0;
    }

    BoyerMooreSearcher<uint32_t> bm(std::move(pattern));

    auto onMatch = [&](std::vector<uint32_t>::iterator it) {
        size_t idx = std::distance(textValues.begin(), it);
        std::cout << tokenPositions[idx].line << ", " << tokenPositions[idx].word << "\n";
        return false;
    };

    bm(textValues, onMatch);
    return 0;
}