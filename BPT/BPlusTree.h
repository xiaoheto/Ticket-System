//
// Created by 43741 on 2025/4/23.
//

#ifndef BPLUSTREE_H
#define BPLUSTREE_H
#include <iostream>
#include "STLite/vector.hpp"
#include <climits>
#include <cstddef>
#include <cstdio>
#include <fstream>
#include <cassert>
using std::string;
using std::fstream;
using std::ifstream;
using std::ofstream;
using std::pair;
using std::string;
using std::ios;
using sjtu::vector;

template<class T,int MAX_NODES = 4000,int M = 150,int Buffer_SIZE = 120>
class BPTree {
private:
    const long long MOD1 = 1e9 + 7;
    const long long MOD2 = 1e9 + 9;

    const long long BASE1 = 307;
    const long long BASE2 = 313;

    using Pair = std::pair<long long,long long>;
    const int sizeofInt = sizeof(int);
    const int leaf_limit = (M + 1) / 2;
    const int internal_limit = M / 2;//e.g. M = 5,internal node has at least 2 data,leaf node has at least 3 data.

    struct Node {
        bool is_leaf = false;
        int size_ = 0;
        int id_ = 0;
        int pre_node = 0;
        int next_node = 0;
        Pair index[M + 1]{};
        int son[M + 2]{};
        Node() = default;
    };
    int sizeofNode = sizeof(Node);

    struct Node_value {
        T values[M + 2]{};
    };
    int sizeofNodeValue = sizeof(Node_value);

    struct  BasicInformation {
        int root_node_id = 0;
        int other_info = 0;
        int empty_node_id[MAX_NODES]{};
    };
    int sizeofBasicInfo = sizeof(BasicInformation);

    Pair GetHash(const string &s) {
        long long ans1 = 0;
        long long ans2 = 0;
        for(auto i:s) {
            ans1 = ans1 * BASE1 + (long long)(i) % MOD1;
            ans2 = ans2 * BASE1 + (long long)(i) % MOD2;
        }
        return {ans1,ans2};
    }

    fstream file,value_file;
    string index_filename,value_filename,filename;

    Node root;
    BasicInformation basic_info;

    BasicInformation ReadBasicInfoDisk() {
        file.seekg(ios::beg);
        BasicInformation temp;
        file.read((char *)&temp,sizeofBasicInfo);
        return temp;
    }

    void WriteBasicInfoDisk(BasicInformation info) {
        file.seekp(ios::beg);
        file.write(reinterpret_cast<char *>(&info),sizeofBasicInfo);
    }

    Node ReadNodeDisk(int pos) {
        file.seekg(sizeofBasicInfo + (pos - 1) * sizeofNode);
        Node node;
        file.read((char *)&node,sizeofNode);
        return node;
    }

    void WriteNodeDisk(int pos,Node node) {
        file.seekp(sizeofBasicInfo + (pos - 1) * sizeofNode,ios::beg);
        file.write(reinterpret_cast<char *>(&node),sizeofNode);
    }

    Node_value ReadNodeValue(int pos) {
        value_file.seekp((pos - 1) * sizeofNodeValue,ios::beg);
        Node_value temp;
        value_file.read((char *)&temp,sizeofNodeValue);
        return temp;
    }

    void WriteNodeValueDisk(int pos,Node value) {
        value_file.seekp((pos - 1) * sizeofNodeValue,ios::beg);
        value_file.write(reinterpret_cast<char *>(&value_file),sizeofNodeValue);
    }

    struct Buffer_pool {
        BasicInformation basic_info;//缓存文件头的数据
        int node_size = 0,value_size = 0;//当前缓存的节点的数目
        bool info_flag = false;
        Node nodes[Buffer_SIZE];
        Node_value values[Buffer_SIZE];

        int node_id[Buffer_SIZE] {},value_id[Buffer_SIZE]{};//记录缓存项的磁盘节点
        int node_time[Buffer_SIZE]{},value_time[Buffer_SIZE]{};//实现缓存置换的时间
        int time_tag;//全局时间技术器
    } buffer;

    void RefreshBasicInfo() {
        WriteBasicInfoDisk(buffer.basic_info);
    }

    void PopNode(int pos) {
        WriteNodeDisk(buffer.node_id[pos],buffer.nodes[pos]);
        buffer.value_size--;
    }

    void InitialiseFile() {
        int temp = 0;
        file.open(index_filename,ios::out | ios::binary);
        file.write(reinterpret_cast<char *>(&temp),sizeofInt);
        file.close();

        value_file.open(value_filename,ios::out | ios::binary);
        value_file.write(reinterpret_cast<char *>(&temp),sizeofInt);
        value_file.close();
    }

    bool CheckFileExist(const string &str) {
        fstream temp(str);
        if (temp.is_open()) {
            temp.close();
            return true;
        }
        return false;
    }

    std::pair<int,int>GetSibling(int current_id,Node parent_node) {
        assert(!parent_node.is_leaf);
        int pos = -1;
        for(int i = 0;i < parent_node.size_;++i) {
            if (current_id == parent_node.son[i]) {
                pos = i;
                break;;
            }
        }
        assert(pos >= 0);
        if (pos == 0) {
            return std::make_pair(0,parent_node.son[1]);
        }
        else if (pos == parent_node.size_) {
            return std::make_pair(parent_node.size_ - 1,0);
        }
        else {
            return std::make_pair(parent_node.son[pos - 1],parent_node.son[pos + 1]);
        }
    }

    std::pair<vector <int>,Node>FindNode(Pair index_hash,T value_) {
    }

    bool BorrowFromPreLeaf(Node pre_node,Node current_node,Node parent_node) {
        if (pre_node.size_ <= leaf_limit) {
            return false;
        }

        Node_value parent_value = ReadNodeValue(parent_node.id_);
        Node_value cur_value = ReadNodeValue(current_node.id);
        Node_value pre_value = ReadNodeValue(pre_node.id);

        for(int i = current_node.size_;i >= 1;--i) {
            current_node.index[]
        }

    }

};
#endif //BPLUSTREE_H
