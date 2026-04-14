#include <iostream>
#include <cstdint>
#include <unordered_map>
#include <cassert>
#include <cctype>
#include <iterator>
#include <concepts>
#include <algorithm>
#include <utility>
#include <new>

template <class T>
class Vector {
private:
    T* data_{};
    size_t size_{};
    size_t capacity_{};

    static T* allocate(size_t capacity) {
        if (capacity == 0)
            return nullptr;
        return static_cast<T*>(operator new[](capacity * sizeof(T)));
    }

    void reallocate(size_t newCapacity) noexcept {
        T* newData = allocate(newCapacity);
        for (size_t i = 0; i < size_; ++i) {
            new(newData + i) T(std::move(data_[i]));
            data_[i].~T();
        }

        operator delete[](data_);
        data_ = newData;
        capacity_ = newCapacity;
    }

    void ensureCapacity(size_t requiredCapacity) {
        if (requiredCapacity <= capacity_)
            return;

        size_t newCapacity = capacity_ == 0 ? 1 : capacity_;
        while (newCapacity < requiredCapacity)
            newCapacity *= 2;
        reallocate(newCapacity);
    }

    void swap(Vector& other) noexcept {
        std::swap(data_, other.data_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

public:
    Vector() = default;

    Vector(const Vector& other) : data_{allocate(other.capacity_)}, size_{other.size_}, capacity_{other.capacity_} {
        for (size_t i = 0; i < size_; ++i)
            new(data_ + i) T(other.data_[i]);
    }

    Vector(Vector&& other) noexcept : data_{other.data_}, size_{other.size_}, capacity_{other.capacity_} {
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }

    template <typename InputIt>
    Vector(InputIt first, InputIt last) : Vector() {
        while (first != last) {
            push_back(*first);
            ++first;
        }
    }

    Vector(size_t size, const T& value) : Vector() {
        reserve(size);
        for (size_t i = 0; i < size; ++i)
            push_back(value);
    }

    Vector& operator=(const Vector& other) {
        if (this == &other)
            return *this;
        Vector copy(other);
        swap(copy);
        return *this;
    }

    Vector& operator=(Vector&& other) noexcept {
        if (this == &other)
            return *this;
        swap(other);
        return *this;
    }

    ~Vector() {
        clear();
        operator delete[](data_);
    }

    size_t size() const {
        return size_;
    }

    size_t capacity() const {
        return capacity_;
    }

    bool empty() const {
        return size_ == 0;
    }

    T* data() {
        return data_;
    }

    const T* data() const {
        return data_;
    }

    T& operator[](size_t index) {
        return data_[index];
    }

    const T& operator[](size_t index) const {
        return data_[index];
    }

    T& back() {
        return data_[size_ - 1];
    }

    const T& back() const {
        return data_[size_ - 1];
    }

    T* begin() { return data_; }
    T* end() { return data_ + size_; }
    const T* begin() const { return data_; }
    const T* end() const { return data_ + size_; }

    void reserve(size_t newCapacity) {
        if (newCapacity <= capacity_)
            return;
        reallocate(newCapacity);
    }

    void resize(size_t newSize, const T& value = T()) {
        ensureCapacity(newSize);
        if (newSize > size_) {
            for (size_t i = size_; i < newSize; ++i)
                new(data_ + i) T(value);
        } else if (newSize < size_) {
            for (size_t i = newSize; i < size_; ++i)
                data_[i].~T();
        }
        size_ = newSize;
    }

    void push_back(const T& value) {
        ensureCapacity(size_ + 1);
        new(data_ + size_) T(value);
        ++size_;
    }

    void push_back(T&& value) {
        ensureCapacity(size_ + 1);
        new(data_ + size_) T(std::move(value));
        ++size_;
    }

    template <typename... Args>
    T& emplace_back(Args&&... args) {
        ensureCapacity(size_ + 1);
        new(data_ + size_) T(std::forward<Args>(args)...);
        return data_[size_++];
    }

    void assign(size_t newSize, const T& value) {
        clear();
        reserve(newSize);
        for (size_t i = 0; i < newSize; ++i)
            push_back(value);
    }


    void pop_back() {
        assert(size_ > 0);
        data_[size_ - 1].~T();
        --size_;
    }

    void clear() {
        for (size_t i = 0; i < size_; ++i)
            data_[i].~T();
        size_ = 0;
    }
};

class String {
private:
    Vector<char> buffer_;

    void ensureInitialized() {
        if (buffer_.empty())
            buffer_.push_back('\0');
    }

    size_t dataSize() const {
        return buffer_.empty() ? 0 : buffer_.size() - 1;
    }

public:
    String() {
        buffer_.push_back('\0');
    }

    explicit String(const char* cstr) : String() {
        if (!cstr)
            return;
        size_t len = 0;
        while (cstr[len] != '\0')
            ++len;
        buffer_.resize(len + 1, '\0');
        for (size_t i = 0; i < len; ++i)
            buffer_[i] = cstr[i];
    }

    String(const String& other) = default;
    String(String&& other) noexcept = default;
    String& operator=(const String& other) = default;
    String& operator=(String&& other) noexcept = default;
    ~String() = default;

    size_t size() const {
        return dataSize();
    }

    size_t capacity() const {
        return buffer_.capacity() == 0 ? 0 : buffer_.capacity() - 1;
    }

    void resize(size_t newSize) {
        ensureInitialized();
        buffer_.resize(newSize + 1, '\0');
        buffer_[newSize] = '\0';
    }

    char* data() {
        ensureInitialized();
        return buffer_.data();
    }

    const char* data() const {
        return buffer_.data();
    }

    const char* c_str() const {
        return data();
    }

    char& operator[](size_t index) {
        return buffer_[index];
    }

    const char& operator[](size_t index) const {
        return buffer_[index];
    }

    bool operator==(const String& other) const {
        if (size() != other.size())
            return false;
        for (size_t i = 0; i < size(); ++i) {
            if (buffer_[i] != other.buffer_[i])
                return false;
        }
        return true;
    }

    bool operator!=(const String& other) const {
        return !(*this == other);
    }

    bool operator==(const char* other) const {
        return *this == String(other);
    }

    friend std::istream& operator>>(std::istream& in, String& str) {
        Vector<char> temp;
        bool started = false;

        while (true) {
            int next = in.peek();
            if (next == std::char_traits<char>::eof())
                break;

            if (std::isspace(static_cast<unsigned char>(next))) {
                if (!started)
                    in.get();
                else
                    break;
                continue;
            }

            started = true;
            temp.push_back(static_cast<char>(in.get()));
        }

        if (!started) {
            in.setstate(std::ios::failbit);
            return in;
        }

        temp.push_back('\0');
        str = String(temp.data());
        return in;
    }

    static bool getline(std::istream& in, String& str, char delimiter = '\n') {
        str.buffer_.clear();
        int c = in.get();
        if (c == EOF)
            return false;

        while (c != delimiter && c != EOF) {
            str.buffer_.push_back(static_cast<char>(c));
            c = in.get();
        }
        str.buffer_.push_back('\0');
        return true;
    }

    void clear() {
        buffer_.clear();
        buffer_.push_back('\0');
    }

    char* begin() { return buffer_.data(); }
    char* end() { return buffer_.data() + buffer_.size(); }
    const char* begin() const { return buffer_.data(); }
    const char* end() const { return buffer_.data() + buffer_.size(); }
};

class NumberIterator {
private:
    const char* ptr_;
    uint32_t currentValue_;
    bool isEnd_;
    bool hasValue_;

    void skipNonDigits() {
        while (ptr_ && *ptr_ && !std::isdigit(static_cast<unsigned char>(*ptr_))) {
            ++ptr_;
        }
    }

    void advance() {
        skipNonDigits();
        if (!ptr_ || !*ptr_ || !std::isdigit(static_cast<unsigned char>(*ptr_))) {
            isEnd_ = true;
            hasValue_ = false;
            return;
        }
        
        uint32_t value = 0;
        while (ptr_ && *ptr_ && std::isdigit(static_cast<unsigned char>(*ptr_))) {
            value = value * 10 + (*ptr_ - '0');
            ++ptr_;
        }
        currentValue_ = value;
        hasValue_ = true;
    }
    
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = uint32_t;
    using difference_type = std::ptrdiff_t;
    using pointer = const uint32_t*;
    using reference = const uint32_t&;

    explicit NumberIterator(const char* str) 
        : ptr_(str), currentValue_(0), isEnd_(false), hasValue_(false) {
        advance();
    }

    NumberIterator() : ptr_(nullptr), currentValue_(0), isEnd_(true), hasValue_(false) {}

    uint32_t operator*() const {
        return currentValue_;
    }
    
    NumberIterator& operator++() {
        advance();
        return *this;
    }

    bool operator!=(const NumberIterator& other) const {
        return isEnd_ != other.isEnd_;
    }

    bool operator==(const NumberIterator& other) const {
        return !(*this != other);
    }
};

template <typename T>
class View {
private:
    const T* data_;
    size_t size_;

public:
    View() : data_(nullptr), size_(0) {}
    View(const T* data, size_t size) : data_(data), size_(size) {}

    View(const Vector<T>& vec) : data_(vec.data()), size_(vec.size()) {}

    View(const T* first, const T* last)
        : data_(first), size_(static_cast<size_t>(last - first)) {}

    const T* data() const { return data_; }
    size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }

    const T& operator[](size_t index) const { return data_[index]; }

    const T* begin() const { return data_; }
    const T* end() const { return data_ + size_; }
};

template <typename T = uint32_t, typename Pred = std::equal_to<T>>
class BoyerMooreSearcher {
    using Diff = std::ptrdiff_t;

    View<T> pattern_;
    std::unordered_map<T, Diff> bad_char_;
    Vector<Diff> good_suffix_;
    Pred pred_;

public:
    explicit BoyerMooreSearcher(const View<T>& pattern, const Pred& pred = Pred())
        : pattern_(pattern), pred_(pred) {
        if (pattern_.empty()) 
            return;
        
        preprocessBadChar();
        preprocessGoodSuffix();
    }

    template <std::random_access_iterator Iter, typename Callback>
    void operator()(const Iter& first, const Iter& last, Callback&& onMatch) const {
        const Diff n = std::distance(first, last);
        const Diff m = pattern_.size();
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
                shift += good_suffix_[0];
            } else {
                Diff bc_shift = 1;
                if (auto it = bad_char_.find(*(first + shift + j)); 
                it != bad_char_.end()) {
                    bc_shift = j - (m - 1 - it->second);
                }
                shift += std::max({(Diff)1, bc_shift, good_suffix_[j]});
            }
        }
    }

private:
    void preprocessBadChar() {
        const Diff m = pattern_.size();
        bad_char_.clear();

        for (Diff i = 0; i < m - 1; ++i) {
            bad_char_[pattern_[i]] = m - 1 - i;
        }
    }

    void preprocessGoodSuffix() {
        const Diff m = pattern_.size();
        good_suffix_.assign(m, m);

        Vector<Diff> suff = computeSuffixes();

        Diff lastPrefixIndex = m;

        for (Diff i = m - 1; i >= 0; --i) {
            if (suff[i] == i + 1) {
                lastPrefixIndex = i + 1;
            }
            good_suffix_[i] = lastPrefixIndex + (m - 1 - i);
        }

        for (Diff i = 0; i < m - 1; ++i) {
            good_suffix_[m - 1 - suff[i]] = m - 1 - i;
        }
    }

    Vector<Diff> computeSuffixes() const {
        const Diff m = pattern_.size();
        Vector<Diff> suff(m, 0);
        suff[m - 1] = m;

        for (Diff i = m - 2, g = m - 1, f = m - 1; i >= 0; --i) {
            if (i > g && suff[i + m - 1 - f] < i - g) {
                suff[i] = suff[i + m - 1 - f];
            } else {
                if (i < g)
                    f = i;

                g = std::min(g, i);
                while (g >= 0 && pred_(pattern_[g], pattern_[g + m - 1 - f]))
                    --g;
                suff[i] = f - g;
            }
        }
        return suff;   
    }
};
