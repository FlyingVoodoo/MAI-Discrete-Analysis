#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <string_view>
#include <algorithm>
#include <limits>

class SuffixTree final {
    struct Node final {
        std::vector<std::pair<char, std::unique_ptr<Node>>> edges;
        Node* parent{nullptr};
        Node* suffix_link{nullptr};
        size_t edge_start{0};
        size_t edge_len{std::string::npos};

        Node() = default;
        Node(Node* par, size_t start, size_t len)
            : parent(par), edge_start(start), edge_len(len) {}

        bool is_leaf() const noexcept {
            return edge_len == std::string::npos;
        }

        std::unique_ptr<Node>* find_edge(char c) noexcept {
            auto it = std::lower_bound(edges.begin(), edges.end(), c,
                [](const std::pair<char, std::unique_ptr<Node>>& edge, char val) {
                    return edge.first < val;
                });

            if (it != edges.end() && it->first == c) {
                return &(it->second);
            }
            return nullptr;
        }

        void add_edge(char c, std::unique_ptr<Node> child) {
            auto it = std::lower_bound(edges.begin(), edges.end(), c,
                [](const std::pair<char, std::unique_ptr<Node>>& edge, char val) {
                    return edge.first < val;
                });
            edges.emplace(it, c, std::move(child));
        }

        Node* min_child(char terminator) const noexcept {
            if (edges.empty()) return nullptr;
            
            auto it = edges.begin();
            if (it->first == terminator) {
                ++it;
            }
            
            return it != edges.end() ? it->second.get() : nullptr;
        }
    };

    struct ActivePoint {
        Node* node{nullptr};
        size_t edge_char{0};
        size_t length{0};
    };

    std::unique_ptr<Node> root_;
    std::string text_;
    ActivePoint ap_;
    size_t remainder_{0};

    static constexpr char kTerminator = '$';

    Node* raw_root() const noexcept { return root_.get(); }

    size_t resolved_edge_len(const Node* n) const noexcept {
        return n->is_leaf() ? text_.size() - n->edge_start : n->edge_len;
    }

    static void link_if_pending(Node*& pending, Node* target) noexcept {
        if (pending) pending->suffix_link = target;
        pending = nullptr;
    }

    void advance_active_point(size_t pos) noexcept {
        if (ap_.node == raw_root() && ap_.length > 0) {
            --ap_.length;
            ap_.edge_char = pos - remainder_ + 1;
        } else {
            ap_.node = (ap_.node->suffix_link && ap_.node->suffix_link != ap_.node)
                ? ap_.node->suffix_link
                : raw_root();
        }
    }

    Node* split_edge(std::unique_ptr<Node>& edge_owner, size_t pos, char new_char) {
        Node* old_child = edge_owner.release();
        auto split_owner = std::make_unique<Node>(old_child->parent, old_child->edge_start, ap_.length);
        Node* split = split_owner.get();

        old_child->edge_start += ap_.length;
        if (!old_child->is_leaf()) {
            old_child->edge_len -= ap_.length;
        }
        old_child->parent = split;

        split->add_edge(text_[old_child->edge_start], std::unique_ptr<Node>(old_child));
        split->add_edge(new_char, std::make_unique<Node>(split, pos, std::string::npos));

        edge_owner = std::move(split_owner);
        split->suffix_link = raw_root();

        return split;
    }

    void extend(size_t pos) {
        char c = text_[pos];
        Node* pending = nullptr;
        ++remainder_;

        while (remainder_ > 0) {
            if (ap_.length == 0) {
                ap_.edge_char = pos;
            }

            char active_ch = text_[ap_.edge_char];
            std::unique_ptr<Node>* edge_ptr = ap_.node->find_edge(active_ch);

            if (!edge_ptr) {
                ap_.node->add_edge(active_ch,
                    std::make_unique<Node>(ap_.node, pos, std::string::npos));
                link_if_pending(pending, ap_.node);
            } else {
                Node* next = edge_ptr->get();
                size_t next_len = resolved_edge_len(next);

                if (ap_.length >= next_len) {
                    ap_.edge_char += next_len;
                    ap_.length -= next_len;
                    ap_.node = next;
                    continue;
                }

                if (text_[next->edge_start + ap_.length] == c) {
                    ++ap_.length;
                    link_if_pending(pending, ap_.node);
                    break;
                }

                Node* split = split_edge(*edge_ptr, pos, c);
                link_if_pending(pending, split);
                pending = split;
            }

            --remainder_;
            advance_active_point(pos);
        }
    }

public:
    SuffixTree() : root_(std::make_unique<Node>()) {
        root_->suffix_link = raw_root();
        root_->parent = raw_root();
        ap_.node = raw_root();
    }

    void build(std::string_view s) {
        text_.reserve(s.size());
        for (char c : s) {
            text_ += c;
            extend(text_.size() - 1);
        }
    }

    void print_min_rotation(size_t n) const {
        const Node* cur = raw_root();
        size_t printed = 0;

        while (printed < n) {
            const Node* next = cur->min_child(kTerminator);
            if (!next) {
                break;
            }

            size_t elen = resolved_edge_len(next);
            size_t to_print = std::min(elen, n - printed);

            std::cout.write(text_.data() + next->edge_start,
                            static_cast<std::streamsize>(to_print));
            printed += to_print;
            cur = next;
        }
        std::cout << '\n';
    }
};

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::string s;
    if (!(std::cin >> s)) return 0;

    SuffixTree tree;
    tree.build(s + s + '$');
    tree.print_min_rotation(s.size());
}