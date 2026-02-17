#include <cstddef>
#include <iostream>
#include <string>
#include <vector>
#include <iomanip>

struct Node {
    size_t offset;
    size_t length;
    int next;
};

const size_t MAX_KEYS = 1000000;
const size_t MAX_NODES = 1000000;

int heads[MAX_KEYS];
int tails[MAX_KEYS];
std::vector<Node> pool;

std::vector<char> text_pool;

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

