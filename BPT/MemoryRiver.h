//
// Created by 43741 on 2025/5/11.
//

#ifndef MEMORYRIVER_H
#define MEMORYRIVER_H
#include <iostream>
#include <fstream>
using std::fstream;
using std::string;
using std::ios;

template<typename T, int info_len = 2>
class MemoryRiver {

    fstream file;
    string filename;
    int sizeofT = sizeof(T);

public:
    MemoryRiver() = default;

    MemoryRiver(const string &file_name) : filename(file_name) {}

    void initialise(const string &FN = "",bool clearFile = false) {
        if (!FN.empty()) {
            filename = FN;
        }
        file.open(filename);
        if (!file || clearFile) {
            file.open(filename, std::ios::out | ios::binary);
            file.close();
            file.open(filename, std::ios::in | std::ios::out);
            int tmp = -1, tmp2 = 0;
            file.write(reinterpret_cast<char *>(&tmp), sizeof(int));
            for (int i = 1; i < info_len; ++i){
                file.write(reinterpret_cast<char *>(&tmp2), sizeof(int));
            }
        }
    }

    void get_info(int &tmp, int n) {
        if (n > info_len) {
            return;
        }
        file.seekg((n - 1) * sizeof(int), std::ios::beg);
        file.read(reinterpret_cast<char *>(&tmp), sizeof(int));
    }

    void write_info(int tmp, int n) {
        if (n > info_len) {
            return;
        }
        file.seekp((n - 1) * sizeof(int), std::ios::beg);
        file.write(reinterpret_cast<char *>(&tmp), sizeof(int));
    }

    int write(T &t) {
        file.seekp(0, std::ios::end);
        int index = file.tellp();
        index = (index - info_len * sizeof(int)) / sizeofT;
        update(t, index);
        return index;
    }

    int get_index() {
        file.seekp(0, std::ios::end);
        int index = file.tellp();
        index = (index - info_len * sizeof(int)) / sizeofT;
        return index;
    }

    void update(T &t, const int index) {
        file.seekp(info_len * sizeof(int) + index * sizeofT);
        file.write(reinterpret_cast<char *>(&t), sizeofT);
    }

    void read(T &t, const int index) {
        file.seekg(info_len * sizeof(int) + index * sizeofT);
        file.read(reinterpret_cast<char *>(&t), sizeofT);
    }

    void end() {
        file.close();
    }
};
#endif //MEMORYRIVER_H
