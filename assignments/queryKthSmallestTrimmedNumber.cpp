#include <vector>
#include <string>
#include <numeric>

using namespace std;

class Solution {
public:
    vector<int> smallestTrimmedNumbers(vector<string>& nums, vector<vector<int>>& queries) {
        int n = nums.size();
        int L = nums[0].length();
        
        vector<int> p(n);
        iota(p.begin(), p.end(), 0);
        
        vector<vector<int>> history(L + 1);
        history[0] = p;

        for (int step = 1; step <= L; ++step) {
            int charIdx = L - step;
            
            vector<vector<int>> buckets(10);
            for (int idx : p) {
                int digit = nums[idx][charIdx] - '0';
                buckets[digit].push_back(idx);
            }
            
            p.clear();
            for (int d = 0; d < 10; ++d) {
                for (int idx : buckets[d]) {
                    p.push_back(idx);
                }
            }
            
            history[step] = p;
        }

        vector<int> result;
        for (auto& q : queries) {
            int k = q[0];
            int trim = q[1];
            result.push_back(history[trim][k - 1]);
        }
        return result;
    }
};