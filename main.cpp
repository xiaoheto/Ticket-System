#include <string>
#include <sstream>
#include <fstream>
#include <cassert>
#include <random>
#include <cstddef>
#include <iostream>
#include <cstring>
using std::string;
using std::fstream;
using std::ifstream;
using std::ofstream;
using std::pair;
using std::string;
using std::ios;
using std::string;
using std::fstream;
using std::ifstream;
using std::ofstream;
#include "BPlusTree.hpp"
#include "MyChar.h"
int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    BPlusTree<MyChar<65>,int> db;

    int n;
    std::cin >> n;
    std::string line;
    std::getline(std::cin, line); // 读掉换行符

    for (int i = 0; i < n; ++i) {
        std::getline(std::cin, line);
        std::stringstream ss(line);
        std::string command;
        MyChar<65> index;
        int value;

        ss >> command;

        if (command == "insert") {
            ss >> index >> value;
            db.insert(index, value);
        } else if (command == "delete") {
            ss >> index >> value;
            db.erase(index, value);
        } else if (command == "find") {
            ss >> index;
            vector<int> result = db.query(index);
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