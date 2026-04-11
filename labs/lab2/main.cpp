#include <bit>
#include <cassert>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <fstream>
#include <iostream>
#include <locale>
#include <utility>
#include <functional>

template <class T>
class Vector {
private:
    T* data_{};
    size_t size_{};
    size_t capacity_{};

    static T* allocate(size_t capacity) {
        if (capacity == 0)
            return nullptr;
        return new T[capacity];
    }

    void reallocate(size_t newCapacity) {
        T* newData = allocate(newCapacity);
        for (size_t i = 0; i < size_; ++i)
            newData[i] = std::move(data_[i]);
        delete[] data_;
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

public:
    Vector() = default;

    Vector(const Vector& other) : data_{allocate(other.capacity_)}, size_{other.size_}, capacity_{other.capacity_} {
        for (size_t i = 0; i < size_; ++i)
            data_[i] = other.data_[i];
    }

    Vector(Vector&& other) noexcept : data_{other.data_}, size_{other.size_}, capacity_{other.capacity_} {
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }

    Vector& operator=(const Vector& other) {
        if (this == &other)
            return *this;
        if (other.size_ > capacity_) {
            Vector copy(other);
            *this = std::move(copy);
            return *this;
        }
        for (size_t i = 0; i < other.size_; ++i)
            data_[i] = other.data_[i];
        size_ = other.size_;
        return *this;
    }

    Vector& operator=(Vector&& other) noexcept {
        if (this == &other)
            return *this;
        delete[] data_;
        data_ = other.data_;
        size_ = other.size_;
        capacity_ = other.capacity_;
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
        return *this;
    }

    ~Vector() {
        delete[] data_;
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

    void reserve(size_t newCapacity) {
        if (newCapacity <= capacity_)
            return;
        reallocate(newCapacity);
    }

    void resize(size_t newSize, const T& value = T()) {
        ensureCapacity(newSize);
        if (newSize > size_) {
            for (size_t i = size_; i < newSize; ++i)
                data_[i] = value;
        }
        size_ = newSize;
    }

    void push_back(const T& value) {
        ensureCapacity(size_ + 1);
        data_[size_++] = value;
    }

    void push_back(T&& value) {
        ensureCapacity(size_ + 1);
        data_[size_++] = std::move(value);
    }

    void pop_back() {
        assert(size_ > 0);
        --size_;
    }

    void clear() {
        size_ = 0;
    }
};

template <class T>
class Stack {
private:
    Vector<T> data_;

public:
    bool empty() const {
        return data_.empty();
    }

    size_t size() const {
        return data_.size();
    }

    void push(const T& value) {
        data_.push_back(value);
    }

    void push(T&& value) {
        data_.push_back(std::move(value));
    }

    void pop() {
        data_.pop_back();
    }

    T& top() {
        return data_.back();
    }

    const T& top() const {
        return data_.back();
    }

    void clear() {
        data_.clear();
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
        char temp[2048];
        in >> temp;
        str = String(temp);
        return in;
    }
};

template <class Key>
struct IdentityNormalizer {
    Key operator()(const Key &key) const {
        return key;
    }
};

struct LowercaseLatinNormalizer {
    String operator()(const String &key) const {
        String normalized = key;
        static const std::locale classic = std::locale::classic();
        for (size_t i = 0; i < normalized.size(); ++i) {
            normalized[i] = std::tolower(normalized[i], classic);
        }
        return normalized;
    }
};

template <class Key, class T, class Digitizer, class Normalizer = IdentityNormalizer<Key>>
class PatriciaTree final {
private:
    static constexpr size_t maxKeyLength{256};

    struct Node {
        std::pair <Key, T> value{};
        Node *left{}, *right{};
        ptrdiff_t diffBit{-1};
        size_t saveId{static_cast<size_t>(-1)};
    };

    Node *root{nullptr};
    size_t size{0};
    Digitizer digitizer{};
    Normalizer normalizer{};

    void clear() {
        if (!root) return;

        Stack<Node*> st;
        Vector<Node*> toDelete;
        toDelete.reserve(size == 0 ? 1 : size);

        root->saveId = 0;
        st.push(root);

        while (!st.empty()) {
            Node* cur = st.top();
            st.pop();

            toDelete.push_back(cur);

            if (cur->left && cur->left->diffBit > cur->diffBit && cur->left->saveId == static_cast<size_t>(-1)) {
                cur->left->saveId = 0;
                st.push(cur->left);
            }

            if (cur->right && cur->right->diffBit > cur->diffBit && cur->right->saveId == static_cast<size_t>(-1)) {
                cur->right->saveId = 0;
                st.push(cur->right);
            }

        }

        for (size_t i = 0; i < toDelete.size(); ++i)
            delete toDelete[i];

        root = nullptr;
        size = 0;
    }

public:
    explicit PatriciaTree(const Digitizer &digitizer = Digitizer(), const Normalizer &normalizer = Normalizer())
        : digitizer{digitizer}, normalizer{normalizer} {}

    ~PatriciaTree() {
        clear();
    }

    PatriciaTree(const PatriciaTree& other) : digitizer{other.digitizer}, normalizer{other.normalizer} {
        if (!other.root) return;

        Vector<Node*> oldNodes;
        Vector<Node*> newNodes;

        Stack<Node*> st;
        st.push(const_cast<Node*>(other.root));

        try {
            while (!st.empty()) {
                Node* cur = st.top();
                st.pop();

                if (cur->saveId != static_cast<size_t>(-1))
                    continue;

                cur->saveId = oldNodes.size();
                oldNodes.push_back(cur);
                newNodes.push_back(new Node{cur->value, nullptr, nullptr, cur->diffBit});

                if (cur->left && cur->left->diffBit > cur->diffBit)
                    st.push(cur->left);
                if (cur->right && cur->right->diffBit > cur->diffBit)
                    st.push(cur->right);
            }

            for (size_t i = 0; i < oldNodes.size(); ++i) {
                newNodes[i]->left = newNodes[oldNodes[i]->left->saveId];
                newNodes[i]->right = newNodes[oldNodes[i]->right->saveId];
            }

            root = newNodes[const_cast<Node*>(other.root)->saveId];
            size = other.size;
        } catch (...) {
            for (size_t i = 0; i < newNodes.size(); ++i)
                delete newNodes[i];
            for (size_t i = 0; i < oldNodes.size(); ++i)
                oldNodes[i]->saveId = static_cast<size_t>(-1);
            throw;
        }

        for (size_t i = 0; i < oldNodes.size(); ++i)
            oldNodes[i]->saveId = static_cast<size_t>(-1);
    }

    PatriciaTree& operator=(const PatriciaTree& other) {
        if (this != &other) {
            PatriciaTree copy(other);
            *this = std::move(copy);
        }
        return *this;
    }

    PatriciaTree(PatriciaTree &&other) noexcept
                : root{std::exchange(other.root, nullptr)},
                    size{std::exchange(other.size, 0)},
                    digitizer{std::move(other.digitizer)},
                    normalizer{std::move(other.normalizer)} {}

    PatriciaTree& operator=(PatriciaTree &&other) noexcept {
        if (this != &other) {
            clear();
            root = std::exchange(other.root, nullptr);
            size = std::exchange(other.size, 0);
            digitizer = std::move(other.digitizer);
            normalizer = std::move(other.normalizer);
        }
        return *this;
    }

    bool insert(const Key &key, const T &value) {
        const Key normalizedKey = normalizer(key);
        if (normalizedKey.size() > maxKeyLength)
            return false;

        if (root == nullptr) {
            root = new Node{{normalizedKey, value}, nullptr, nullptr, -1};
            root->left = root;
            root->right = root;
            ++size;
            return true;
        }

        Node *current = root;
        Node *next = root->left;
        while (next->diffBit > current->diffBit) {
            current = next;
            next = digitizer(normalizedKey, next->diffBit) ? next->right : next->left;
        }
        if (next->value.first == normalizedKey)
            return false;

        const ptrdiff_t newDiffBit = digitizer(normalizedKey, next->value.first);

        current = root;
        next = root->left;
        while (next->diffBit > current->diffBit && next->diffBit < newDiffBit) {
            current = next;
            next = digitizer(normalizedKey, next->diffBit) ? next->right : next->left;
        }

        Node *newNode = new Node{{normalizedKey, value}, nullptr, nullptr, newDiffBit};
        if (digitizer(normalizedKey, newDiffBit)) {
            newNode->right = newNode;
            newNode->left = next;
        } else {
            newNode->left = newNode;
            newNode->right = next;
        }

        if (digitizer(normalizedKey, current->diffBit)) {
            current->right = newNode;
        } else {
            current->left = newNode;
        }

        ++size;
        return true;
    }

    bool erase(const Key &key) {
        const Key normalizedKey = normalizer(key);
        if (normalizedKey.size() > maxKeyLength)
            return false;
        if (root == nullptr)
            return false;

        if (root->left == root) {
            if (root->value.first != normalizedKey)
                return false;
            delete root;
            root = nullptr;
            --size;
            return true;
        }

        Node *upParent = nullptr;
        Node *upToDelete = root;
        Node *deleteNode = root->left;

        do {
            upParent = upToDelete;
            upToDelete = deleteNode;
            deleteNode = digitizer(normalizedKey, deleteNode->diffBit)
                ? deleteNode->right
                : deleteNode->left;
        } while (deleteNode->diffBit > upToDelete->diffBit);

        if (deleteNode->value.first != normalizedKey)
            return false;

        Node *upToUp = nullptr;
        Node *temp = upToDelete;

        do {
            upToUp = temp;
            temp = digitizer(upToDelete->value.first, temp->diffBit) 
                ? temp->right
                : temp->left;
        } while (temp->diffBit > upToUp->diffBit);

        std::swap(deleteNode->value, upToDelete->value);
        if (upToUp->left == upToDelete)
            upToUp->left = deleteNode;
        else
            upToUp->right = deleteNode;

        Node *replacement = (upToDelete->left == deleteNode)
            ? upToDelete->right
            : upToDelete->left;

        if (upParent->left == upToDelete)
            upParent->left = replacement;
        else
            upParent->right = replacement;

        delete upToDelete;
        --size;

        if (size == 1) {
            root->left = root;
            root->right = root;
        }
        
        return true;
    }

    std::pair<bool, const Node*> find(const Key &key) const {
        const Key normalizedKey = normalizer(key);
        if (normalizedKey.size() > maxKeyLength)
            return {false, nullptr};

        if (root == nullptr)
            return {false, nullptr};

        Node *current = root;
        Node *next = root->left;

        while (next->diffBit > current->diffBit) {
            current = next;
            next = digitizer(normalizedKey, next->diffBit) ? next->right : next->left;
        }
        return {next->value.first == normalizedKey, next};

    }

    bool get(const Key &key, T &value) const {
        const auto [exists, node] = find(key);
        if (!exists)
            return false;
        value = node->value.second;
        return true;
    }

    bool save(const String& path) const {
        std::ofstream out(path.c_str(), std::ios::binary);
        if (!out)
            return false;

        out.write(reinterpret_cast<const char*>(&size), sizeof(size));
        if (size == 0 || root == nullptr)
            return true;

        Vector<Node*> queue;
        queue.reserve(size);

        root->saveId = 0;
        queue.push_back(root);

        size_t head = 0;
        while (head < queue.size()) {
            Node* cur = queue[head++];

            out.write(reinterpret_cast<const char*>(&cur->diffBit), sizeof(cur->diffBit));

            size_t kLen = cur->value.first.size();
            out.write(reinterpret_cast<const char*>(&kLen), sizeof(kLen));
            out.write(cur->value.first.data(), static_cast<std::streamsize>(kLen));
            out.write(reinterpret_cast<const char*>(&cur->value.second), sizeof(T));

            size_t leftId = static_cast<size_t>(-1);
            if (cur->left != nullptr) {
                if (cur->left->saveId != static_cast<size_t>(-1)) {
                    leftId = cur->left->saveId;
                } else {
                    leftId = queue.size();
                    cur->left->saveId = leftId;
                    queue.push_back(cur->left);
                }
            }
            out.write(reinterpret_cast<const char*>(&leftId), sizeof(leftId));

            size_t rightId = static_cast<size_t>(-1);
            if (cur->right != nullptr) {
                if (cur->right->saveId != static_cast<size_t>(-1)) {
                    rightId = cur->right->saveId;
                } else {
                    rightId = queue.size();
                    cur->right->saveId = rightId;
                    queue.push_back(cur->right);
                }
            }
            out.write(reinterpret_cast<const char*>(&rightId), sizeof(rightId));
        }

        const bool ok = out.good();
        for (size_t i = 0; i < queue.size(); ++i)
            queue[i]->saveId = static_cast<size_t>(-1);

        return ok;
    }

    bool load(const String& path) {
        std::ifstream in(path.c_str(), std::ios::binary);
        if (!in)
            return false;

        size_t newSize = 0;
        if (!in.read(reinterpret_cast<char*>(&newSize), sizeof(newSize)))
            return false;

        PatriciaTree tmp(digitizer, normalizer);
        tmp.size = newSize;

        if (newSize == 0) {
            *this = std::move(tmp);
            return true;
        }

        struct LoadPatch {
            Node** place;
            size_t id;
        };

        Vector<Node*> nodes;
        nodes.reserve(newSize);
        for (size_t i = 0; i < newSize; ++i)
            nodes.push_back(new Node());

        Vector<LoadPatch> patches;
        patches.reserve(newSize * 2);

        bool error = false;
        for (size_t i = 0; i < newSize; ++i) {
            Node* cur = nodes[i];
            ptrdiff_t diffBit;

            if (!in.read(reinterpret_cast<char*>(&diffBit), sizeof(diffBit))) {
                error = true;
                break;
            }

            size_t kLen;
            if (!in.read(reinterpret_cast<char*>(&kLen), sizeof(kLen))) {
                error = true;
                break;
            }

            String key;
            key.resize(kLen);
            if (!in.read(key.data(), static_cast<std::streamsize>(kLen))) {
                error = true;
                break;
            }

            T value;
            if (!in.read(reinterpret_cast<char*>(&value), sizeof(T))) {
                error = true;
                break;
            }

            size_t leftId;
            size_t rightId;
            if (!in.read(reinterpret_cast<char*>(&leftId), sizeof(leftId)) ||
                !in.read(reinterpret_cast<char*>(&rightId), sizeof(rightId))) {
                error = true;
                break;
            }

            if (leftId != static_cast<size_t>(-1)) {
                if (leftId >= newSize) {
                    error = true;
                    break;
                }
                patches.push_back({&cur->left, leftId});
            }

            if (rightId != static_cast<size_t>(-1)) {
                if (rightId >= newSize) {
                    error = true;
                    break;
                }
                patches.push_back({&cur->right, rightId});
            }

            cur->diffBit = diffBit;
            cur->value = {key, value};
        }

        if (error) {
            for (size_t i = 0; i < nodes.size(); ++i)
                delete nodes[i];
            return false;
        }

        for (size_t i = 0; i < patches.size(); ++i)
            *patches[i].place = nodes[patches[i].id];

        tmp.root = nodes[0];
        *this = std::move(tmp);
        return true;
    }
};

class LatinDigitizer {
private:
    static constexpr int charBits{5};
    static inline const std::locale
&classic{std::locale::classic()};

public:
    bool operator()(
        const String &str,
        const ptrdiff_t bit
    ) const noexcept {
        if (bit < 0)
            return false;
        const ptrdiff_t charIndex = bit / charBits;
        if (charIndex >= static_cast<ptrdiff_t>(str.size()))
            return false;
    
        assert(std::islower(str[charIndex], classic));
        return bool(str[charIndex] >> (bit % charBits) & 1);
    }

    ptrdiff_t operator()(
        const String &first,
        const String &second
    ) const noexcept {
        const ptrdiff_t lengthFirst{static_cast<ptrdiff_t>(first.size())},
lengthSecond{static_cast<ptrdiff_t>(second.size())};
    if (lengthFirst > lengthSecond)
        return (*this)(second, first);
    
    for (ptrdiff_t i{0}; i < lengthFirst; ++i) {
        assert(std::islower(first[i], classic) && std::islower(second[i], classic));
        if (first[i] != second[i])
            return std::countr_zero(static_cast<unsigned char>(first[i] ^ second[i])) + i * charBits;
    }

    if (lengthFirst == lengthSecond)
            return -1;
    
    return std::countr_zero(static_cast<unsigned char>(second[lengthFirst])) + lengthFirst * charBits;
    }
};

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    using Dictionary = PatriciaTree<String, std::uint64_t, LatinDigitizer, LowercaseLatinNormalizer>;
    Dictionary dictionary;

    String command;
    while (std::cin >> command) {
        try {
            if (command == "+") {
                String word;
                std::uint64_t value = 0;
                if (!(std::cin >> word >> value)) {
                    std::cout << "NoSuchWord\n";
                    continue;
                }
                std::cout << (dictionary.insert(word, value) ? "OK\n" : "Exist\n");
                continue;
            }

            if (command == "-") {
                String word;
                if (!(std::cin >> word)) {
                    std::cout << "NoSuchWord\n";
                    continue;
                }
                std::cout << (dictionary.erase(word) ? "OK\n" : "NoSuchWord\n");
                continue;
            }

            if (command == "!") {
                String action;
                String path;
                if (!(std::cin >> action >> path)) {
                    std::cout << "ERROR: Invalid command\n";
                    continue;
                }

                if (action == "Save") {
                    dictionary.save(path);
                    std::cout << "OK\n";
                } else if (action == "Load") {
                    dictionary.load(path);
                    std::cout << "OK\n";
                } else {
                    std::cout << "ERROR: Invalid command\n";
                }
                continue;
            }

            std::uint64_t value = 0;
            if (dictionary.get(command, value))
                std::cout << "OK: " << value << '\n';
            else
                std::cout << "NoSuchWord\n";
        } catch (const std::exception &error) {
            std::cout << "ERROR: " << error.what() << '\n';
        }
    }

    return 0;
}