#include <iostream>
#include <string>
#include <sstream>
#include "BPT/BPlusTree.h"
#include "vector.hpp"
using sjtu::vector;

void quick_sort(vector<int> &arr, int left, int right) {
    if (left >= right) return;
    int pivot = arr[left + (right - left) / 2];
    int i = left, j = right;
    while (i <= j) {
        while (arr[i] < pivot) ++i;
        while (arr[j] > pivot) --j;
        if (i <= j) {
            int tmp = arr[i];
            arr[i] = arr[j];
            arr[j] = tmp;
            ++i;
            --j;
        }
    }
    if (left < j) quick_sort(arr, left, j);
    if (i < right) quick_sort(arr, i, right);
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    BPlusTree<int> db("ticket_system");
    db.Initialise("ticket_system", 1);

    int n;
    std::cin >> n;
    std::string line;
    std::getline(std::cin, line);

    for (int i = 0; i < n; ++i) {
        std::getline(std::cin, line);
        std::stringstream ss(line);
        std::string command, index;
        int value;

        ss >> command;

        if (command == "insert") {
            ss >> index >> value;
            db.Insert(index, value);
        } else if (command == "delete") {
            ss >> index >> value;
            db.Erase(index, value);
        } else if (command == "find") {
            ss >> index;
            vector<int> result = db.Search(index);
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
