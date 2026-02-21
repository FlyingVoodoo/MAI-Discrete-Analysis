#include <cstddef>
#include <iostream>
#include <string>
#include <iomanip>

template <typename T>
class Vector {
private:
    T* data_;
    size_t size_;
    size_t capacity_;

    void grow(size_t new_cap) {
        T* new_data = new T[new_cap];
        for (size_t i = 0; i < size_; ++i) {
            new_data[i] = data_[i];
        }
        delete[] data_;
        data_ = new_data;
        capacity_ = new_cap;
    }

public:
    Vector() : data_(nullptr), size_(0), capacity_(0) {}
    Vector(const Vector&) = delete;
    Vector& operator=(const Vector&) = delete;

    ~Vector() { delete[] data_; }

    void reserve(size_t cap) {
        if (cap > capacity_) {
            grow(cap);
        }
    }

    void push_back(const T& val) {
        if (size_ == capacity_) {
            grow(capacity_ == 0 ? 1 : capacity_ * 2);
        }
        data_[size_++] = val;
    }

    template <typename It>
    void insert(T* pos, It first, It last) {
        size_t count = 0;
        for (It it = first; it != last; ++it) ++count;
        if (size_ + count > capacity_) {
            size_t new_cap = capacity_ == 0 ? count : capacity_;
            while (new_cap < size_ + count) new_cap *= 2;
            grow(new_cap);
        }
        for (It it = first; it != last; ++it) {
            data_[size_++] = *it;
        }
    }

    T* end() { return data_ + size_; }
    T* data() { return data_; }
    size_t size() const { return size_; }
    T& operator[](size_t idx) { return data_[idx]; }
    const T& operator[](size_t idx) const { return data_[idx]; }
};

struct Node {
    size_t offset;
    size_t length;
    int next;
};

const size_t MAX_KEYS = 1000000;
const size_t MAX_NODES = 1000000;

int heads[MAX_KEYS];
int tails[MAX_KEYS];
Vector<Node> pool;

Vector<char> text_pool;

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::fill(heads, heads + MAX_KEYS, -1);
    std::fill(tails, tails + MAX_KEYS, -1);

    int key;
    int pool_idx = 0;
    std::string buffer;

    pool.reserve(MAX_NODES);
    text_pool.reserve(80 * 1024 * 1024);

    while (std::cin >> key) {
        std::cin.ignore();
        if (!std::getline(std::cin, buffer)) {
            break;
        }
        size_t offset = text_pool.size();
        text_pool.insert(text_pool.end(), buffer.begin(), buffer.end());

        int current_node_idx = static_cast<int>(pool.size());

        pool.push_back({offset, buffer.size(), -1});

        if (heads[key] == -1) {
            heads[key] = current_node_idx;
        } else {
            pool[tails[key]].next = current_node_idx;
        }
        tails[key] = current_node_idx;
    }

    for (int i = 0; i <= MAX_KEYS - 1; ++i) {
        int cur_index = heads[i];
        while (cur_index != -1) {
            const Node& node = pool[cur_index];
            std::cout << std::setfill('0') << std::setw(6) << i << "\t";
            std::cout.write(text_pool.data() + node.offset, node.length);
            std::cout << "\n";
            cur_index = node.next;
        }
 
    }
    return 0;
}