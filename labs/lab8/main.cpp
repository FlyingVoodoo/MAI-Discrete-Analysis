#include <iostream>
#include <vector>
#include <cstddef>
#include <optional>

class TopologicalSorter final {
    using usize = std::size_t;

    std::vector<std::vector<usize>> edges_;
    std::vector<usize> in_degree_;

public:
    explicit TopologicalSorter(usize vertex_count)
        : edges_(vertex_count), in_degree_(vertex_count, 0) {}

    void add_edge(usize from, usize to) {
        edges_[from].push_back(to);
        ++in_degree_[to];
    }

    std::optional<std::vector<usize>> sort() const {
        auto in_degree = in_degree_;
        std::vector<usize> order;
        order.reserve(edges_.size());

        for (usize vertex = 0; vertex < edges_.size(); ++vertex) {
            if (in_degree[vertex] == 0) {
                order.push_back(vertex);
            }
        }

        for (usize head = 0; head < order.size(); ++head) {
            const usize vertex = order[head];

            for (const usize next : edges_[vertex]) {
                if (--in_degree[next] == 0) {
                    order.push_back(next);
                }
            }
        }

        if (order.size() != edges_.size()) {
            return std::nullopt;
        }

        return order;
    }
};

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::size_t n = 0;
    std::size_t m = 0;
    if (!(std::cin >> n >> m)) return 0;

    TopologicalSorter sorter(n);
    for (std::size_t i = 0; i < m; ++i) {
        std::size_t from = 0;
        std::size_t to = 0;
        std::cin >> from >> to;
        sorter.add_edge(from - 1, to - 1);
    }

    const auto order = sorter.sort();
    if (!order) {
        std::cout << -1 << '\n';
        return 0;
    }

    for (std::size_t i = 0; i < order->size(); ++i) {
        if (i > 0) std::cout << ' ';
        std::cout << (*order)[i] + 1;
    }
    std::cout << '\n';

    return 0;
}
