#include <iostream>
#include <string>
#include <sstream>

#include "STLite/vector.hpp"
#include "BPT/BPlusTree.hpp";
using std::cin;
using std::cout;
using sjtu::vector;

int n;
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    cout.tie(nullptr);

    BPlusTree<int> tree;
    cin >> n;
    string line;
    std::getline(cin,line);

    while(n--) {
        std::getline(cin,line);
        std::stringstream ss(line);
        string command;
        char index[65];
        int value;

        ss >> command;

        if (command == "insert") {
            ss >> index >> value;
            tree.insert(index,value);
        } else if (command == "delete") {
            ss >> index >> value;
            tree.erase(index,value);
        } else if (command == "find") {
            ss >> index;
            vector<int> result = tree.query(index);
            if (result.empty()) {
                std::cout << "null\n";
            } else {
                quick_sort(result, 0, result.size() - 1);
                for (int j = 0; j < result.size(); ++j) {
                    if (j) std::cout << ' ';
                    std::cout << result[j];
                }
                std::cout << '\n';
            }
        }
    }
    return 0;
}