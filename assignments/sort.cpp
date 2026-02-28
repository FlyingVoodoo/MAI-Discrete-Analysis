#include <algorithm>
#include <vector>
#include <iterator>

using namespace std;

class Solution final {
private:
    template<class Iter>
    using Value = iterator_traits<Iter>::value_type;

    template<class T>
    static constexpr T ceilDiv(const T x, const T y) noexcept {
        return x / y + (isSameSign(x, y) && x % y != T{0});
    }

    template<class T>
    static constexpr bool isOdd(const T value) noexcept {
        return bool(value & T{1});
    }

    template<class T>
    static constexpr T isSameSign(const T x, const T y) noexcept {
        return (x ^ y) >= T{0};
    }

    template<class T>
    static constexpr size_t getBits(
        const T &value,
        const size_t idx,
        const size_t count
    ) noexcept {
        return size_t((value >> idx) & ((T{1} << count) - T{1}));
    }

    template<class InIter, class CountIter, class OutIter, class Digitizer>
    static constexpr void countingSort(
        const InIter inFirst,
        const InIter inLast,
        const CountIter countFirst,
        const CountIter countLast,
        const OutIter outFirst,
        Digitizer &&digitizer
    ) noexcept {
        fill(countFirst, countLast, 0UZ);

        for_each(inFirst, inLast,
            [countFirst, &digitizer](
                const auto &element
            ) constexpr noexcept -> void {
                const auto digit{digitizer(element)};
                ++countFirst[digit];
            }
        );

        inclusive_scan(countFirst, countLast, countFirst);

        for_each(reverse_iterator(inLast), reverse_iterator(inFirst),
            [countFirst, outFirst, &digitizer](
                auto &element
            ) constexpr noexcept -> void {
                const auto digit{digitizer(element)};
                const auto idx{--countFirst[digit]};
                outFirst[idx] = move(element);
            }
        );
    }

    template<class Iter>
    static constexpr void radixSort(
        Iter first,
        Iter last,
        const size_t bitsStep,
        const size_t bitsLength
    ) {
        vector<size_t> count(1UZ << bitsStep);
        const auto countFirst{begin(count)}, countLast{end(count)};

        const size_t length(distance(first, last));
        vector<Value<Iter>> buffer(length);
        const auto bufferFirst{begin(buffer)}, bufferLast{end(buffer)};

        for (auto idx{0UZ}; idx < bitsLength; idx += bitsStep) {
            const auto step{min(bitsStep, bitsLength - idx)};
            const auto getDigit{
                [idx, step](const auto &element) constexpr noexcept -> auto {
                    return getBits(element, idx, step);
                }
            };

            if (isOdd(idx / bitsStep))
                countingSort(
                    bufferFirst, bufferLast,
                    countFirst, countLast,
                    first,
                    getDigit
                );
            else
                countingSort(
                    first, last,
                    countFirst, countLast,
                    bufferFirst,
                    getDigit
                );
        }

        if (isOdd(ceilDiv(bitsLength, bitsStep)))
            move(bufferFirst, bufferLast, first);
    }

public:
    constexpr vector<int> sortArray(vector<int> &nums) const noexcept {
        static constexpr auto bound{50'000};

        for (auto &num : nums)
            num += bound;

        radixSort(begin(nums), end(nums), 9, 17);

        for (auto &num : nums)
            num -= bound;

        return move(nums);
    }
};
