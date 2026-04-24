#include <string>
#include <vector>
#include <algorithm>
#include <iterator>
#include <utility>

using namespace std;

class Solution {
private:
    struct VirtualString {
        string_view source;
        int virtual_size;

        constexpr VirtualString(string_view s) 
            : source(s), virtual_size(static_cast<int>(s.length() * 2 + 3)) {}

        struct Iterator {
            using iterator_category = std::random_access_iterator_tag;
            using value_type        = char;
            using difference_type   = int;
            using pointer           = const char*;
            using reference         = char;

            const VirtualString* vs;
            int index;

            constexpr char operator*() const {
                if (index <= 0) return '^';
                if (index >= vs->virtual_size - 1) return '$';
                if (index % 2 == 0) return '#';
                return vs->source[(index - 1) / 2];
            }

            constexpr Iterator& operator++() { ++index; return *this; }
            constexpr Iterator& operator--() { --index; return *this; }
            constexpr Iterator operator+(int n) const { return {vs, index + n}; }
            constexpr Iterator operator-(int n) const { return {vs, index - n}; }
            constexpr int operator-(const Iterator& other) const { return index - other.index; }
            constexpr bool operator==(const Iterator& other) const { return index == other.index; }
            constexpr auto operator<=>(const Iterator& other) const = default;
        };

        Iterator begin() const { return {this, 0}; }
        Iterator end() const { return {this, virtual_size}; }
    };

    template<class Iter, class Func>
    static constexpr void findPalindromes(
        Iter first,
        Iter last,
        Func &&callback
    ) {
        int n = static_cast<int>(std::distance(first, last));
        if (n < 3)
            return;
        std::vector<int> p(n, 0);
        int center = 0; 
        int right = 0;

        for (int i = 1; i < n - 1; ++i) {
            if (i < right) {
                int mirror = 2 * center - i;
                p[i] = std::min(right - i, p[mirror]);
            }

            while (*(first + i - p[i] - 1) == *(first + i + p[i] + 1)) {
                ++p[i];
            }

            if (i + p[i] > right) {
                center = i;
                right = i + p[i];
            }

            callback(i, p[i]);
        }
    }

public:
    constexpr string longestPalindrome(string_view str) const {
        if (str.length() <= 1) 
            return string(str);

        VirtualString vs(str);

        int maxLen = 0;
        int startInOrig = 0;

        findPalindromes(vs.begin(), vs.end(), [&](auto i, auto radius) {
            if (radius > maxLen) {
                maxLen = radius;
                startInOrig = (i - radius) / 2;
            }
        });

        return string(str.substr(startInOrig, maxLen));
    }
};