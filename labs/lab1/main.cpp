#include <cstddef>
#include <iostream>
#include <utility>
#include <iomanip>
#include <iomanip>

constexpr size_t INDEX_RANGE = 1000000;

template <typename T>
class Vector {
private:
    T* data_;
    size_t size_;
    size_t capacity_;

    void grow(size_t new_cap) {
        T* new_data = new T[new_cap];
        for (size_t i = 0; i < size_; ++i) {
            new_data[i] = std::move(data_[i]);
        }
        delete[] data_;
        data_ = new_data;
        capacity_ = new_cap;
    }

public:
    Vector() : data_(nullptr), size_(0), capacity_(0) {}

    Vector(const Vector& other) : data_(new T[other.capacity_]), size_(other.size_), capacity_(other.capacity_) {
        for (size_t i = 0; i < size_; ++i) {
            data_[i] = other.data_[i];
        }
    }

    Vector& operator=(Vector&& other) noexcept {
        if (this != &other) {
            delete[] data_;
            data_ = std::exchange(other.data_, nullptr);
            size_ = std::exchange(other.size_, 0);
            capacity_ = std::exchange(other.capacity_, 0);
        }
        return *this;
    }
    
    Vector& operator=(const Vector& other) {
        if (this != &other) {
            Vector temp(other);
            std::swap(data_, temp.data_);
            std::swap(size_, temp.size_);
            std::swap(capacity_, temp.capacity_);
        }
        return *this;
    }

    Vector(Vector&& other) noexcept : data_(std::exchange(other.data_, nullptr)), size_(std::exchange(other.size_, 0)), capacity_(std::exchange(other.capacity_, 0)) {}

    ~Vector() { delete[] data_; }

    void clear() { size_ = 0; }

    void push_back(const T& val) {
        if (size_ == capacity_) {
            grow(capacity_ == 0 ? 1 : capacity_ * 2);
        }
        data_[size_++] = val;
    }

    void resize(size_t new_size) {
        if (new_size > capacity_) {
            grow(new_size);
        }
        size_ = new_size;
    }

    T* begin() { return data_; }
    T* end() { return data_ + size_; }
    T* data() { return data_; }
    size_t size() const { return size_; }
    T& operator[](size_t idx) { return data_[idx]; }
    const T& operator[](size_t idx) const { return data_[idx]; }
};

class String {
private:
    Vector<char> buffer_;
public:
    void push_back(char c) {
        buffer_.push_back(c);
    }

    size_t size() const {
        return buffer_.size();
    }

    char& operator[](size_t idx) {
        return buffer_[idx];
    }

    void clear() {
        buffer_.clear();
    }

    bool getline(std::istream& in, char delimiter = '\n') {
        buffer_.clear();
        char c;
        while (in.get(c)) {
            if (c == delimiter) {
                return true;
            }
            push_back(c);
        }
        return size() > 0;
    }
};

template <typename StringType>
size_t stringToInt(const StringType& str) {
    size_t result = 0;
    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] < '0' || str[i] > '9') return INDEX_RANGE;
        result = result * 10 + (str[i] - '0');
    }
    return result;
}


template <typename InIter, typename OutIter, typename GetKey>
void countingSort(
    InIter inFirst, 
    InIter inLast, 
    OutIter outFirst, 
    size_t keyCount, 
    GetKey getKey
) {
    Vector<size_t> count;
    count.resize(keyCount);

    for (size_t i = 0; i < keyCount; ++i) {
        count[i] = 0;
    }

    for (InIter it = inFirst; it != inLast; ++it) {
        ++count[getKey(*it)];
    }
    for (size_t i = 1; i < keyCount; ++i) {
        count[i] += count[i - 1];
    }
    for (InIter it = inLast; it != inFirst; ) {
        --it;
        const size_t key = getKey(*it);
        count[key]--;
        outFirst[count[key]] = std::move(*it);
    }
}


int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(NULL);
    struct EntryPair {
        int key;
        String value;
    };

    Vector<EntryPair> entries;
    Vector<EntryPair> sortedEntries;
    int key;
    String value;

    while (std::cin >> key) {
        std::cin.ignore();
        if (!value.getline(std::cin)) {
            break;
        }
        if (value.size() == 0) continue;
        entries.push_back({key, std::move(value)});
    }

    sortedEntries.resize(entries.size());

    auto getKey = [](const EntryPair& entry) -> size_t {
        return static_cast<size_t>(entry.key);
    };

    countingSort(entries.begin(), entries.end(), sortedEntries.begin(), INDEX_RANGE, getKey);

    for (size_t i = 0; i < sortedEntries.size(); ++i) {
        std::cout << std::setfill('0') << std::setw(6) << sortedEntries[i].key << '\t';
        for (size_t j = 0; j < sortedEntries[i].value.size(); ++j)
            std::cout << sortedEntries[i].value[j];
        std::cout << '\n';
    }
    return 0;
}