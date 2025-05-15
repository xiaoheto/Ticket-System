#include <string>
#include <sstream>
#include <fstream>
#include <random>
#include <iostream>
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
#include "BPlusTree.h"
#include "MyChar.h"

void QuickSort(vector<int> &arr, int left, int right) {
    if (left >= right) return;
    int pivot = arr[left + (right - left) / 2];
    int i = left, j = right;
    while (i <= j) {
        while (arr[i] < pivot) ++i;
        while (arr[j] > pivot) --j;
        if (i <= j) {
            std::swap(arr[i], arr[j]);
            ++i;
            --j;
        }
    }
    if (left < j) {
        QuickSort(arr, left, j);
    }
    if (i < right) {
        QuickSort(arr, i, right);
    }
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    BPTree<MyChar<65>,int> db;

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
                QuickSort(result, 0, result.size() - 1);
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
