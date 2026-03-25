template <class key, class T, class Digitizer>
class PatriciaTree final {
private:
    struct Node {
        std::pair <key, T> value{};
        Node *left{}, *right{};
        ptrdiff_t diffBit{-1};
    }
    Node *root{};
    size_t size{0};
    Digitizer digitizer{};

    void clear(Node *node) {
        if (node->left->diffBit > node->diffBit) {
            clear(node->left);
        }
        if (node->right->diffBit > node->diffBit) {
            clear(node->right);
        }
        delete node;
    }

public:
    explicit PatriciaTree(const Digitizer &digitizer = Digitizer()) : digitizer{digitizer} {
        root = new Node();
        root->diffBit = -1;
        root->left = root;
        root->right = root;
    }

    ~PatriciaTree() {
        clear(root);
    }

    PatriciaTree(const PatriciaTree &other) = delete;
    PatriciaTree& operator=(const PatriciaTree &other) = delete;

    PatriciaTree(PatriciaTree &&other) noexcept
        : root{std::exchange(other.root, nullptr)}, size{std::exchange(other.size, 0)}, digitizer{std::move(other.digitizer)} {}

    PatriciaTree& operator=(PatriciaTree &&other) noexcept {
        if (this != &other) {
            clear(root);
            root = std::exchange(other.root, nullptr);
            size = std::exchange(other.size, 0);
            digitizer = std::move(other.digitizer);
        }
        return *this;
    }

    bool insert(const Key &key, const T &value) {
        // TODO: implement insert
    }

    bool erase(const Key &key) {
        // TODO: implement erase
    }

    std::pair<bool, Node*> find(const Key &key) const { 
        Node *current = root;
        Node *next = root->left;

        while (next->diffBit > current->diffBit) {
            current = next;
            next = digitizer(key, next->diffBit) ? next->right : next->left;
        }
        return {next->value.first == key, next};
    };
};

class LatinDigitizer {
private:
    static constexpr int charBits{5};
    static inline const locale
&classic{locale::classic()};

public:
    constexpr bool operator()(
        const string &str,
        const ptrdiff_t bit
    ) const noexcept {
        const ptrdiff_t charIndex = bit / charBits;
        if (charIndex >= ssize(str))
            return false;
    
        assert(isLower(str[charIndex], classic));
        return bool(str[charIndex] >> (bit % charBits) & 1);
    }

    constexpr ptrdiff_t operator()(
        const string &first,
        const string &second
    ) const noexcept {
        const auto lengthFirst{ssize(first)},
lengthSecond{ssize(second)};
    if (lengthFirst > lengthSecond)
        return (*this)(second, first);
    
    for (auto i{0Z}; i < lengthFirst; ++i) {
        assert(isLower(first[i], classic) && isLower(second[i], classic));
        if (first[i] != second[i])
            return countr_zero(first[i] ^ second[i]) + i * charBits;
    }

    if (lengthFirst == lengthSecond)
        return -1;
    
    return countr_zero(first[lengthFirst] ^ second[lengthFirst]) + lengthFirst * charBits;
    }
};
