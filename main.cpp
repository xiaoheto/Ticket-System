#include <iostream>
#include <cstring>
#include "BPT/BPlusTree.h"

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(false);
    std::cout.tie(false);
    int n;
    std::cin >> n;
    std::string command,index;
    int value;
    while(n--) {
        std::cin >> command;
        if (command == "insert") {
            std::cin >> index >> value;
        }
        else if (command == "delete") {
            std::cin >> index >> value;
        }
        else {
            std::cin >> index;
        }
    }
}