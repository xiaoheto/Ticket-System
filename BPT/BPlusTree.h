//
// Created by 43741 on 2025/4/23.
//

#ifndef BPLUSTREE_H
#define BPLUSTREE_H
#include "STLite/vector.hpp"
#include <fstream>
#include <cassert>
#include <random>
using std::string;
using std::fstream;
using std::ifstream;
using std::ofstream;
using std::pair;
using std::string;
using std::ios;
using sjtu::vector;

template<class T, int MAX_NODES = 4000, int M = 150, int Buffer_size = 120>
class BPlusTree {
private:
    using HashIndex = std::pair<long long, long long>;
    const int sizeofInt = sizeof(int);
    const int leaf_limit = (M + 1) / 2;
    const int internal_limit = M / 2; //e.g. M = 5,internal node has at least 2 data,leaf node has at least 3 data.

    struct Node {
        bool is_leaf = false;
        int id_ = 0;
        int pre_node = 0; //the pos of pre_node;
        int nxt_node = 0;
        int size_ = 0;

        HashIndex index[M + 1]{};
        int son[M + 2]{};

        Node() = default;
    };

    int sizeofNode = sizeof(Node);

    struct BasicInfo {
        int root_id_ = 0;
        int other_info_ = 0;
        int empty_id_[MAX_NODES];
    };

    int sizeofBasicInfo = sizeof(BasicInfo);


    struct NodeValue {
        T values[M + 2];
    };

    int sizeofValue = sizeof(NodeValue);

    HashIndex GetHash(const string &str) {
        long long ans1 = 0;
        long long ans2 = 0;
        const long long MOD1 = 1e9 + 7, MOD2 = 1e9 + 9;
        const long long BASE1 = 307, BASE2 = 313;

        for (auto i: str) {
            ans1 = (ans1 * BASE1 + static_cast<long long>(i)) % MOD1;
            ans2 = (ans2 * BASE2 + static_cast<long long>(i)) % MOD2;
        }
        return {ans1, ans2};
    }

    Node root;
    BasicInfo basic_information;

    fstream index_file, value_file; //in the start of index_file,there contains basic_info
    string filename, index_filename, value_filename;

    //operations related to the disk
    Node ReadNodeDisk(int pos) {
        index_file.seekg(sizeofBasicInfo + (pos - 1) * sizeofNode, ios::beg);
        Node node;
        index_file.read(reinterpret_cast<char *>(&node), sizeofNode);
        return node;
    }

    void WriteNodeDisk(int pos, Node node) {
        index_file.seekp(sizeofBasicInfo + (pos - 1) * sizeofNode, ios::beg);
        index_file.write(reinterpret_cast<char *>(&node), sizeofNode);
    }

    NodeValue ReadValueDisk(int pos) {
        value_file.seekg((pos - 1) * sizeofValue, ios::beg);
        NodeValue value;
        value_file.read(reinterpret_cast<char *>(&value), sizeofValue);
        return value;
    }

    void WriteValueDisk(int pos, NodeValue value) {
        value_file.seekp((pos - 1) * sizeofValue, ios::beg);
        value_file.write(reinterpret_cast<char *>(&value), sizeofValue);
    }

    BasicInfo ReadBasicInfoDisk() {
        index_file.seekg(ios::beg);
        BasicInfo info;
        index_file.read(reinterpret_cast<char *>(&info), sizeofBasicInfo);
        return info;
    }

    void WriteBasicInfoDisk(BasicInfo info) {
        index_file.seekp(ios::beg);
        index_file.write(reinterpret_cast<char *>(&info), sizeofBasicInfo);
    }

    struct Buffer {
        bool flag = false;
        BasicInfo basic_info;
        Node nodes[Buffer_size];
        NodeValue values[Buffer_size];
        int node_size = 0, value_size = 0;

        int node_id[Buffer_size]{}, value_id[Buffer_size]{};
        int node_time[Buffer_size]{}, value_time[Buffer_size]{};
        int time_tag;
    } buffer_;

    int FindBufferEmpty() {
        int ans = 0;
        for (int i = 0; i < MAX_NODES; ++i) {
            if (basic_information.empty_id_[i] > 0) {
                ans = basic_information.empty_id_[i];
                basic_information.empty_id_[i] = 0;
                break;
            }
        }
        assert(ans > 0);
        return ans;
    }

    void DeleteNodeBuffer(int pos) {
        WriteNodeDisk(buffer_.node_id[pos], buffer_.nodes[pos]);
        --buffer_.node_size;
        if (pos != buffer_.node_size) {
            buffer_.node_id[pos] = buffer_.node_id[buffer_.node_size];
            buffer_.node_time[pos] = buffer_.node_time[buffer_.node_size];
            buffer_.nodes[pos] = buffer_.nodes[buffer_.node_size];
        }
    }

    void DeleteValueBuffer(int pos) {
        WriteValueDisk(buffer_.value_id[pos], buffer_.values[pos]);
        --buffer_.value_size;
        if (pos != buffer_.value_size) {
            buffer_.value_id[pos] = buffer_.value_id[buffer_.value_size];
            buffer_.value_time[pos] = buffer_.value_time[buffer_.value_size];
            buffer_.values[pos] = buffer_.values[buffer_.value_size];
        }
    }

    void ChangeNodeBuffer(int pos, int id_, Node node) {
        buffer_.node_id[pos] = id_;
        buffer_.node_time[pos] = buffer_.time_tag;
        buffer_.nodes[pos] = node;
    }

    void ChangeValueBuffer(int pos, int id_, NodeValue value) {
        buffer_.value_id[pos] = id_;
        buffer_.value_time[pos] = buffer_.time_tag;
        buffer_.values[pos] = value;
    }

    bool CheckNodeInBuffer(int id_) {
        for (int i = 0; i < buffer_.node_size; ++i) {
            if (buffer_.node_id[i] == id_) {
                return true;
            }
        }
        return false;
    }

    bool CheckValueInBuffer(int id_) {
        for (int i = 0; i < buffer_.value_size; ++i) {
            if (buffer_.value_id[i] == id_) {
                return true;
            }
        }
        return false;
    }

    void AddNodeBuffer(int id_, Node node) {
        ++buffer_.time_tag;
        if (buffer_.node_size < Buffer_size) {
            ChangeNodeBuffer(buffer_.node_size, id_, node);
            ++buffer_.node_size;
        } else {
            int oldest_time = buffer_.node_time[0];
            int toDeleteIdx = 0;
            for (int i = 1; i < buffer_.node_size; ++i) {
                if (buffer_.node_time[i] < oldest_time) {
                    oldest_time = buffer_.node_time[i];
                    toDeleteIdx = i;
                }
            }
            DeleteNodeBuffer(toDeleteIdx);
            ChangeNodeBuffer(toDeleteIdx, id_, node);
            ++buffer_.node_size;
        }
    }
    void AddValueBuffer(int id_, NodeValue value) {
        ++buffer_.time_tag;
        if (buffer_.value_size < Buffer_size) {
            ChangeValueBuffer(buffer_.value_size, id_, value);
            ++buffer_.value_size;
        } else {
            int oldest_time = buffer_.value_time[0];
            int toDeleteIdx = 0;
            for (int i = 1; i < buffer_.value_size; ++i) {
                if (buffer_.value_time[i] < oldest_time) {
                    oldest_time = buffer_.value_time[i];
                    toDeleteIdx = i;
                }
            }
            DeleteValueBuffer(toDeleteIdx);
            ChangeValueBuffer(toDeleteIdx, id_, value);
            ++buffer_.value_size;
        }
    }


    void UpdateNodeInBuffer(int id_,Node node) {
        ++buffer_.time_tag;
        for(int i = 0;i < buffer_.node_size;++i) {
            if (buffer_.node_id[i] == id_) {
                buffer_.node_time[i] = buffer_.time_tag;
                buffer_.nodes[i] = node;
                break;
            }
        }
    }
    void UpdateValueInBuffer(int id_,NodeValue value) {
        ++buffer_.time_tag;
        for(int i = 0;i < buffer_.value_size;++i) {
            if (buffer_.value_id[i] == id_) {
                buffer_.value_time[i] = buffer_.time_tag;
                buffer_.values[i] = value;
                break;
            }
        }
    }

    Node GetNodeBuffer(int id_) {
        ++buffer_.time_tag;
        for(int i = 0;i < buffer_.node_size;++i) {
            if (buffer_.node_id[i] == id_) {
                buffer_.node_time[i] = buffer_.time_tag;
                return buffer_.nodes[i];
            }
        }
    }
    NodeValue GetValueBuffer(int id_) {
        ++buffer_.time_tag;
        for(int i = 0;i < buffer_.value_size;++i) {
            if (buffer_.value_id[i] == id_) {
                buffer_.value_time[i] = buffer_.time_tag;
                return buffer_.values[i];
            }
        }
    }

    BasicInfo ReadBasicInfo() {
        if (!buffer_.flag) {
            buffer_.flag = true;
            buffer_.basic_info = ReadBasicInfoDisk();
        }
        return buffer_.basic_info;
    }

    void WriteBasicInfo(BasicInfo info) {
        buffer_.basic_info = info;
        WriteBasicInfoDisk(info);
    }

    Node ReadNode(int pos) {
        if (CheckNodeInBuffer(pos)) {
            return GetNodeBuffer(pos);
        }
        else {
            Node node = ReadNodeDisk(pos);
            AddNodeBuffer(pos,node);
            return node;
        }
    }
    void WriteNode(int pos,Node node) {
        if (CheckNodeInBuffer(pos)) {
            UpdateNodeInBuffer(pos,node);
        }
        else {
            AddNodeBuffer(pos,node);
        }
    }

    NodeValue ReadValue(int pos) {
        if (CheckValueInBuffer(pos)) {
            return GetValueBuffer(pos);
        }
        else {
            NodeValue value = ReadValueDisk(pos);
            AddValueBuffer(pos,value);
            return value;
        }
    }
    void WriteValue(int pos,NodeValue value) {
        if (CheckValueInBuffer(pos)) {
            UpdateValueInBuffer(pos,value);
        }
        else {
            AddValueBuffer(pos,value);
        }
    }

    void UpdateNodeAndValue(int pos,Node node,NodeValue value) {
        WriteNode(pos,node);
        WriteValue(pos,value);
    }

    void InitialiseFile() {
        int temp = 0;
        index_file.open(index_filename,ios::out | ios::binary);
        index_file.write(reinterpret_cast<char *>(&temp),sizeofInt);
        index_file.close();

        value_file.open(value_filename,ios::out | ios::binary);
        index_file.write(reinterpret_cast<char *>(&temp),sizeofInt);
        value_file.close();
    }

    pair<int,int>GetSiblings(int cur_id,Node parent_node) {
        assert(!parent_node.is_leaf);
        int pos = -1;
        for(int i = 0;i <= parent_node.size_;++i) {
            if (parent_node.son[i] == cur_id) {
                pos = i;
                break;
            }
        }

        if (pos == 0) {
            return std::make_pair(0,parent_node.son[1]);
        }
        else if (pos == parent_node.size_) {
            return std::make_pair(parent_node.son[parent_node.size_ - 1],0);
        }
        else {
            return std::make_pair(parent_node.son[pos - 1],parent_node.son[pos + 1]);
        }
    }

    bool BorrowFromPreLeaf(Node pre_node, Node cur_node, Node parent_node) {
        if (pre_node.size_ <= leaf_limit) return false;

        NodeValue cur_value = ReadValue(cur_node.id_);
        NodeValue pre_value = ReadValue(pre_node.id_);
        NodeValue parent_value = ReadValue(parent_node.id_);

        for (int i = cur_node.size_; i >= 1; --i) {
            cur_node.index[i] = cur_node.index[i - 1];
            cur_node.son[i] = cur_node.son[i - 1];
            cur_value.values[i] = cur_value.values[i - 1];
        }

        cur_node.index[0] = pre_node.index[pre_node.size_ - 1];
        cur_node.son[0] = pre_node.son[pre_node.size_ - 1];
        cur_value.values[0] = pre_value.values[pre_node.size_ - 1];

        ++cur_node.size_;
        --pre_node.size_;

        pre_node.index[pre_node.size_] = {};
        pre_node.son[pre_node.size_] = 0;
        pre_value.values[pre_node.size_] = T();

        int pos = -1;
        for (int i = 0; i < parent_node.size_; ++i) {
            if (parent_node.son[i + 1] == cur_node.id_) {
                pos = i;
                break;
            }
        }
        assert(pos >= 0);

        parent_node.index[pos] = cur_node.index[0];
        parent_value.values[pos] = cur_value.values[0];

        UpdateNodeAndValue(cur_node.id_, cur_node, cur_value);
        UpdateNodeAndValue(pre_node.id_, pre_node, pre_value);
        UpdateNodeAndValue(parent_node.id_, parent_node, parent_value);

        return true;
    }


    bool BorrowFromNextLeaf(Node next_node, Node cur_node, Node parent_node) {
        if (next_node.size_ <= leaf_limit) {
            return false;
        }

        NodeValue cur_value = ReadValue(cur_node.id_);
        NodeValue parent_value = ReadValue(parent_node.id_);
        NodeValue next_value = ReadValue(next_node.id_);

        cur_node.index[cur_node.size_] = next_node.index[0];
        cur_node.son[cur_node.size_ + 1] = next_node.son[0];
        cur_value.values[cur_node.size_] = next_value.values[0];
        ++cur_node.size_;

        for (int i = 0; i < next_node.size_ - 1; ++i) {
            next_node.index[i] = next_node.index[i + 1];
            next_node.son[i] = next_node.son[i + 1];
            next_value.values[i] = next_value.values[i + 1];
        }
        --next_node.size_;

        next_node.index[next_node.size_] = {};
        next_node.son[next_node.size_] = 0;
        next_value.values[next_node.size_] = T();

        int pos = -1;
        for (int i = 0; i < parent_node.size_; ++i) {
            if (parent_node.son[i + 1] == next_node.id_) {
                pos = i;
                break;
            }
        }
        assert(pos >= 0);
        parent_node.index[pos] = next_node.index[0];
        parent_value.values[pos] = next_value.values[0];

        UpdateNodeAndValue(cur_node.id_, cur_node, cur_value);
        UpdateNodeAndValue(next_node.id_, next_node, next_value);
        UpdateNodeAndValue(parent_node.id_, parent_node, parent_value);

        return true;
    }


    bool BorrowFromPreInternal(Node pre_node,Node cur_node,Node parent_node) {
        if (pre_node.size_ <= internal_limit) {
            return false;
        }

        NodeValue pre_value = ReadValue(pre_node.id_);
        NodeValue cur_value = ReadValue(cur_node.id_);
        NodeValue parent_value = ReadValue(parent_node.id_);

        int pos = -1;
        for(int i = 0;i < parent_node.size_;++i) {
            if (parent_node.son[i + 1] == cur_node.id_) {
                pos = i;
                break;
            }
        }
        assert(pos >= 0);

        cur_node.index[0] = parent_node.index[pos];
        cur_value.values[0] = parent_value.values[pos];
        parent_node.index[pos] = pre_node.index[pre_node.size_ - 1];
        parent_value.values[pos] = pre_value.values[pre_node.size_ - 1];

        for(int i = cur_node.size_;i >= 1;--i) {
            cur_node.index[i] = cur_node.index[i - 1];
            cur_value.values[i] = cur_value.values[i - 1];
            cur_node.son[i + 1] = cur_node.son[i];
        }

        cur_node.son[1] = cur_node.son[0];
        cur_node.son[0] = pre_node.son[pre_node.size_];

        ++cur_node.size_;
        --pre_node.size_;

        UpdateNodeAndValue(cur_node.id_,cur_node,cur_value);
        UpdateNodeAndValue(pre_node.id_,pre_node,pre_value);
        UpdateNodeAndValue(parent_node.id_,parent_node,parent_value);
        return true;
    }

    bool BorrowFromNextInternal(Node cur_node, Node next_node, Node parent_node) {
        if (next_node.size_ <= internal_limit) return false;

        NodeValue cur_value = ReadValue(cur_node.id_);
        NodeValue parent_value = ReadValue(parent_node.id_);
        NodeValue next_value = ReadValue(next_node.id_);

        int pos = -1;
        for (int i = 0; i < parent_node.size_; ++i) {
            if (parent_node.son[i + 1] == next_node.id_) {
                pos = i;
                break;
            }
        }
        assert(pos >= 0);

        cur_node.index[cur_node.size_] = parent_node.index[pos];
        cur_value.values[cur_node.size_] = parent_value.values[pos];
        cur_node.son[cur_node.size_ + 1] = next_node.son[0];

        parent_node.index[pos] = next_node.index[0];
        parent_value.values[pos] = next_value.values[0];

        for (int i = 0; i < next_node.size_ - 1; ++i) {
            next_node.index[i] = next_node.index[i + 1];
            next_node.son[i] = next_node.son[i + 1];
            next_value.values[i] = next_value.values[i + 1];
        }
        next_node.son[next_node.size_ - 1] = next_node.son[next_node.size_];

        --next_node.size_;
        ++cur_node.size_;

        UpdateNodeAndValue(cur_node.id_, cur_node, cur_value);
        UpdateNodeAndValue(next_node.id_, next_node, next_value);
        UpdateNodeAndValue(parent_node.id_, parent_node, parent_value);

        return true;
    }

    int CombineWithPreLeaf(Node pre_node, Node current_node, Node parent_node) {
        NodeValue cur_value = ReadValue(current_node.id_);
        NodeValue pre_value = ReadValue(pre_node.id_);
        NodeValue parent_value = ReadValue(parent_node.id_);

        int next_id = current_node.nxt_node;
        if (next_id) {
            Node next_node = ReadNode(next_id);
            next_node.pre_node = pre_node.id_;
            WriteNode(next_node.id_, next_node);
        }

        for (int i = 0; i < current_node.size_; ++i) {
            pre_node.index[pre_node.size_ + i] = current_node.index[i];
            pre_node.son[pre_node.size_ + i] = current_node.son[i];
            pre_value.values[pre_node.size_ + i] = cur_value.values[i];
        }

        pre_node.size_ += current_node.size_;
        pre_node.nxt_node = current_node.nxt_node;

        basic_information.empty_id_[current_node.id_ - 1] = current_node.id_;

        int pos = -1;
        for (int i = 0; i < parent_node.size_; ++i) {
            if (parent_node.son[i + 1] == current_node.id_) {
                pos = i;
                break;
            }
        }
        assert(pos >= 0);

        UpdateNodeAndValue(pre_node.id_, pre_node, pre_value);

        return pos;
    }

    int CombineWithPreInternal(Node pre_node, Node current_node, Node parent_node) {
        NodeValue cur_value = ReadValue(current_node.id_);
        NodeValue pre_value = ReadValue(pre_node.id_);
        NodeValue parent_value = ReadValue(parent_node.id_);

        int pos = -1;
        for (int i = 0; i < parent_node.size_; ++i) {
            if (parent_node.son[i + 1] == current_node.id_) {
                pos = i;
                break;
            }
        }
        assert(pos >= 0);

        int next_id = current_node.nxt_node;
        if (next_id) {
            Node next_node = ReadNode(next_id);
            next_node.pre_node = pre_node.id_;
            WriteNode(next_id, next_node);
        }

        pre_node.nxt_node = next_id;

        int old_size = pre_node.size_;
        pre_node.index[old_size] = parent_node.index[pos];
        pre_value.values[old_size] = parent_value.values[pos];

        for (int i = 0; i < current_node.size_; ++i) {
            pre_node.index[old_size + i + 1] = current_node.index[i];
            pre_value.values[old_size + i + 1] = cur_value.values[i];
            pre_node.son[old_size + i + 1] = current_node.son[i];
        }

        pre_node.son[old_size + current_node.size_ + 1 - 1] = current_node.son[current_node.size_];
        pre_node.size_ += current_node.size_ + 1;

        basic_information.empty_id_[current_node.id_ - 1] = current_node.id_;
        UpdateNodeAndValue(pre_node.id_, pre_node, pre_value);

        return pos;
    }

    pair<vector<int>, Node> FindNode(HashIndex index, T value) {
        vector<int> track;
        basic_information = ReadBasicInfo();

        if (basic_information.root_id_ == 0) {
            return {{}, Node()};
        }

        int cur_id = basic_information.root_id_;
        Node cur_node = ReadNode(cur_id);
        track.push_back(cur_id);

        bool found = false;

        while (!cur_node.is_leaf) {
            int pos = cur_node.size_;
            bool loaded_value = false;
            NodeValue node_value;

            for (int i = 0; i < cur_node.size_; ++i) {
                if (index < cur_node.index[i]) {
                    pos = i;
                    break;
                }
                if (index == cur_node.index[i]) {
                    if (!loaded_value) {
                        node_value = ReadValue(cur_node.id_);
                        loaded_value = true;
                    }
                    if (value < node_value.values[i]) {
                        pos = i;
                        break;
                    }
                }
            }

            cur_id = cur_node.son[pos];
            cur_node = ReadNode(cur_id);
            track.push_back(cur_id);
        }

        NodeValue leaf_value = ReadValue(cur_node.id_);
        for (int i = 0; i < cur_node.size_; ++i) {
            if (index == cur_node.index[i] && value == leaf_value.values[i]) {
                found = true;
                break;
            }
        }

        if (found) {
            track.push_back(-1);
        }
        return {track, cur_node};
    }

    bool CheckFileExist(const string &name) {
        fstream temp(name);
        if (temp.is_open()) {
            temp.close();
            return true;
        }
        return false;
    }

    int Allocate() {
        int ans = FindBufferEmpty();
        WriteBasicInfo(basic_information);
        return ans;
    }

    void InsertNode(Node cur_node,HashIndex index,T value,int pt) {
        NodeValue cur_value = ReadValue(cur_node.id_);
        int pos = cur_node.size_;
        for(int i = 0;i < cur_node.size_;++i) {
            if (cur_node.index[i] > index || cur_node.index[i] == index && cur_value.values[i] > value) {
                pos = i;
                break;
            }
        }

        for(int i = cur_node.size_;i > pos;--i) {
            cur_node.index[i] = cur_node.index[i - 1];
            cur_node.son[i + 1] = cur_node.son[i];
            cur_value.values[i] = cur_value.values[i - 1];
        }

        cur_node.index[pos] = index;
        cur_node.son[pos + 1] = pt;
        cur_value.values[pos] = value;

        UpdateNodeAndValue(cur_node.id_,cur_node,cur_value);
    }

    void EraseInternal(Node cur_node,int pos,const vector<int>&track,int cnt) {
        NodeValue cur_value = ReadValue(cur_node.id_);
        T toDelete = cur_value.values[pos];

        for(int i = pos;i < cur_node.size_ - 1;++i) {
            cur_node.index[i] = cur_node.index[i + 1];
            cur_node.son[i + 1] = cur_node.son[i + 2];
            cur_value.values[i] = cur_value.values[i + 1];
        }
        --cur_node.size_;

        if (basic_information.root_id_ == cur_node.id_) {
            if (cur_node.size_ == 0) {
                basic_information.root_id_ = cur_node.son[0];
                basic_information.empty_id_[cur_node.id_ - 1] = cur_node.id_;
                WriteBasicInfo(basic_information);
                return;
            }

            UpdateNodeAndValue(cur_node.id_,cur_node,cur_value);
            return;
        }
        UpdateNodeAndValue(cur_node.id_,cur_node,cur_value);
        assert(cnt >= 1);
        Node parent_node,sib_node;
        NodeValue parent_value;

        if (cur_node.size_ < internal_limit) {
            parent_node = ReadNode(track[cnt - 1]);
            pair<int,int>siblings = GetSiblings(cur_node.id_,parent_node);

            bool borrow_flag = false;
            if (siblings.first > 0) {
                sib_node = ReadNode(siblings.first);
                borrow_flag = BorrowFromPreInternal(sib_node,cur_node,parent_node);
            }

            if (!borrow_flag && siblings.second > 0) {
                sib_node = ReadNode(siblings.second);
                borrow_flag = BorrowFromNextInternal(cur_node,sib_node,parent_node);
            }

            if (!borrow_flag) {
                int internal_pos = -1;
                if (siblings.first > 0) {
                    sib_node = ReadNode(siblings.first) ;
                    internal_pos = CombineWithPreInternal(sib_node,cur_node,parent_node);
                }
                else {
                    assert(siblings.second > 0);
                    sib_node = ReadNode(siblings.second);
                    internal_pos = CombineWithPreInternal(cur_node,sib_node,parent_node);
                }

                EraseInternal(parent_node, internal_pos, track, track.size() - 2);
            }
        }
    }

public:
    void ClearBuffer() {
        WriteBasicInfoDisk(buffer_.basic_info);
        for(int i = 0;i < buffer_.node_size;++i) {
            WriteNodeDisk(buffer_.node_id[i],buffer_.nodes[i]);
        }

        for(int i = 0;i < buffer_.value_size;++i) {
            WriteValueDisk(buffer_.value_id[i],buffer_.values[i]);
        }

        buffer_.time_tag = 1;
        buffer_.node_size = buffer_.value_size = 0;
        buffer_.flag = false;
    }

    bool Initialise(string FN = "", bool clear = false) {
        if (!FN.empty()) filename = FN;

        index_filename = filename + "index_file";
        value_filename = filename + "value_file";

        if (CheckFileExist(index_filename) && CheckFileExist(value_filename) && !clear) {
            index_file.open(index_filename, ios::in | ios::out | ios::binary);
            value_file.open(value_filename, ios::in | ios::out | ios::binary);
            return false; //already exist
        }
        InitialiseFile();

        BasicInfo temp;
        temp.root_id_ = 0;
        for (int i = 1; i <= MAX_NODES; ++i) {
            temp.empty_id_[i - 1] = i;
        }
        index_file.open(index_filename, ios::in | ios::out | ios::binary);
        assert(index_file.is_open());
        WriteBasicInfoDisk(temp);

        value_file.open(value_filename, ios::in | ios::out | ios::binary);
        assert(value_file.is_open());

        return true;
    }

    vector<T> Search(const string &ind) {
        HashIndex hash = GetHash(ind);
        vector<T> ans = {};
        basic_information = ReadBasicInfo();
        if (basic_information.root_id_ == 0) {
            return ans;
        }
        Node cur_node = ReadNode(basic_information.root_id_);
        while (!cur_node.is_leaf) {
            int pos = cur_node.size_;
            for (int i = 0; i < cur_node.size_; ++i) {
                if (hash <= cur_node.index[i]) {
                    pos = i;
                    break;
                }
            }
            cur_node = ReadNode(cur_node.son[pos]);
        }

        while (cur_node.size_ > 0 && cur_node.index[cur_node.size_ - 1] < hash && cur_node.nxt_node > 0) {
            cur_node = ReadNode(cur_node.nxt_node);
        }

        while (true) {
            NodeValue value = ReadValue(cur_node.id_);

            for (int i = 0; i < cur_node.size_; ++i) {
                if (cur_node.index[i] == hash) {
                    ans.push_back(value.values[i]);
                } else if (cur_node.index[i] > hash) {
                    return ans;
                }
            }

            if (cur_node.nxt_node > 0) {
                cur_node = ReadNode(cur_node.nxt_node);
            } else break;
        }

        return ans;
    }

    bool Empty() {
        basic_information = ReadBasicInfo();
        return basic_information.root_id_ == 0;
    }

    bool Insert(const string &str, T val) {
        HashIndex hash = GetHash(str);
        basic_information = ReadBasicInfo();

        if (Empty()) {
            basic_information.root_id_ = Allocate();
            Node new_node;
            NodeValue new_value;
            new_node.id_ = basic_information.root_id_;
            new_node.size_ = 1;
            new_node.is_leaf = true;
            new_node.index[0] = hash;
            new_node.son[0] = 0;
            new_value.values[0] = val;
            UpdateNodeAndValue(new_node.id_, new_node, new_value);
            WriteBasicInfo(basic_information);
            return true;
        }

        auto [track, cur_node] = FindNode(hash, val);
        if (track.back() == -1) return false;

        int cnt = track.size() - 1;
        HashIndex to_insert_index = hash;
        T to_insert_value = val;
        int to_insert_ptr = 0;

        while (true) {
            NodeValue cur_value = ReadValue(cur_node.id_);
            if (cur_node.size_ < M) {
                InsertNode(cur_node, to_insert_index, to_insert_value, to_insert_ptr);
                break;
            }

            int pos = cur_node.size_;
            for (int i = 0; i < cur_node.size_; ++i) {
                if (cur_node.index[i] > to_insert_index ||
                    (cur_node.index[i] == to_insert_index && cur_value.values[i] > to_insert_value)) {
                    pos = i;
                    break;
                    }
            }

            for (int i = cur_node.size_; i > pos; --i) {
                cur_node.index[i] = cur_node.index[i - 1];
                cur_node.son[i + 1] = cur_node.son[i];
                cur_value.values[i] = cur_value.values[i - 1];
            }

            cur_node.index[pos] = to_insert_index;
            cur_node.son[pos + 1] = to_insert_ptr;
            cur_value.values[pos] = to_insert_value;
            ++cur_node.size_;

            Node new_node;
            NodeValue new_value;
            int next_id = cur_node.nxt_node;
            new_node.id_ = Allocate();

            if (cur_node.is_leaf) {
                int split = (M + 1) / 2;
                new_node.size_ = cur_node.size_ - split;
                cur_node.size_ = split;
                for (int i = 0; i < new_node.size_; ++i) {
                    new_node.index[i] = cur_node.index[split + i];
                    new_node.son[i] = cur_node.son[split + i];
                    new_value.values[i] = cur_value.values[split + i];
                }
                new_node.is_leaf = true;
                new_node.nxt_node = next_id;
                new_node.pre_node = cur_node.id_;
                cur_node.nxt_node = new_node.id_;
                to_insert_index = new_node.index[0];
                to_insert_value = new_value.values[0];
                to_insert_ptr = new_node.id_;
            } else {
                int split = M / 2;
                new_node.size_ = cur_node.size_ - split - 1;
                for (int i = 0; i < new_node.size_; ++i) {
                    new_node.index[i] = cur_node.index[split + 1 + i];
                    new_node.son[i] = cur_node.son[split + 1 + i];
                    new_value.values[i] = cur_value.values[split + 1 + i];
                }
                new_node.son[new_node.size_] = cur_node.son[cur_node.size_];
                to_insert_index = cur_node.index[split];
                to_insert_value = cur_value.values[split];
                to_insert_ptr = new_node.id_;
                new_node.is_leaf = false;
                cur_node.size_ = split;
            }

            if (next_id) {
                Node next_node = ReadNode(next_id);
                next_node.pre_node = new_node.id_;
                WriteNode(next_node.id_, next_node);
            }

            new_node.nxt_node = next_id;
            new_node.pre_node = cur_node.id_;

            UpdateNodeAndValue(cur_node.id_, cur_node, cur_value);
            UpdateNodeAndValue(new_node.id_, new_node, new_value);

            if (cur_node.id_ == basic_information.root_id_) {
                Node new_root;
                NodeValue new_root_value;
                new_root.id_ = Allocate();
                new_root.size_ = 1;
                new_root.is_leaf = false;
                new_root.index[0] = to_insert_index;
                new_root_value.values[0] = to_insert_value;
                new_root.son[0] = cur_node.id_;
                new_root.son[1] = to_insert_ptr;
                basic_information.root_id_ = new_root.id_;
                UpdateNodeAndValue(new_root.id_, new_root, new_root_value);
                WriteBasicInfo(basic_information);
                break;
            }

            --cnt;
            cur_node = ReadNode(track[cnt]);
        }

        WriteBasicInfo(basic_information);
        return true;
    }


    bool Erase(const string&str,T val) {
        basic_information = ReadBasicInfo();
        HashIndex hash = GetHash(str);
        auto[track,cur_node] = FindNode(hash,val);
        NodeValue cur_val = ReadValue(cur_node.id_);
        if (track.back() != -1) {
            return false;
        }

        track.pop_back();
        int pos = -1;
        for(int i = 0;i < cur_node.size_;++i) {
            if (hash == cur_node.index[i] && val == cur_val.values[i]) {
                pos = i;
                break;
            }
        }
        assert(pos >= 0);
        for(int i = pos;i < cur_node.size_ - 1;++i) {
            cur_node.index[i] = cur_node.index[i + 1];
            cur_node.son[i] = cur_node.son[i + 1];
            cur_val.values[i] = cur_val.values[i + 1];
        }
        --cur_node.size_;

        if (basic_information.root_id_ == cur_node.id_) {
            if (Empty()) {
                basic_information.root_id_ = 0;
                basic_information.empty_id_[cur_node.id_ - 1] = cur_node.id_;
                WriteBasicInfo(basic_information);
                return true;
            }

            UpdateNodeAndValue(cur_node.id_,cur_node,cur_val);
            return true;
        }

        UpdateNodeAndValue(cur_node.id_,cur_node,cur_val);

        assert(track.size() >= 2);
        if (pos == 0 && cur_node.pre_node != 0){
            HashIndex substitution_index = cur_node.index[0];
            T substitution_val = cur_val.values[0];
            Node tmp_node; NodeValue tmp_value;
            int sub_flag = 0;
            for (int i = int(track.size()) - 2; i >= 0; i--){
                tmp_node = ReadNode(track[i]);
                tmp_value = ReadValue(track[i]);
                for (int j = 0; j < tmp_node.size_; j++)
                    if (tmp_node.index[j] == hash && tmp_value.values[j] == val){
                        sub_flag = 1;
                        tmp_node.index[j] = substitution_index;
                        tmp_value.values[j] = substitution_val;
                        UpdateNodeAndValue(tmp_node.id_, tmp_node, tmp_value);
                        break;
                    }
                if (sub_flag) break;
            }
            assert(sub_flag);
        }

        Node parent_node;
        Node sib_node;
        NodeValue parent_values;

        if (cur_node.size_ < leaf_limit){
            assert(track[track.size() - 2] > 0);
            parent_node = ReadNode(track[track.size() - 2]);

            std::pair<int, int> siblings = GetSiblings(cur_node.id_, parent_node); //pre and nxt
            int borrow_flag = 0;
            if (siblings.first > 0){
                assert(cur_node.pre_node == siblings.first);
                sib_node = ReadNode(siblings.first);
                borrow_flag = BorrowFromPreLeaf(sib_node, cur_node, parent_node);
            }
            if (!borrow_flag && siblings.second > 0) {
                assert(cur_node.nxt_node == siblings.second);
                sib_node = ReadNode(siblings.second);
                borrow_flag = BorrowFromNextLeaf(cur_node, sib_node, parent_node);
            }
            if (!borrow_flag){
                int internal_pos = -1;
                if (siblings.first > 0){
                    sib_node = ReadNode(siblings.first);
                    internal_pos = CombineWithPreLeaf(sib_node, cur_node, parent_node);
                } else {
                    sib_node = ReadNode(siblings.second);
                    internal_pos = CombineWithPreLeaf(cur_node, sib_node, parent_node);
                }
                EraseInternal(parent_node, internal_pos, track, track.size() - 2);
            }
        }
        WriteBasicInfo(basic_information);
        return 1;
    }

    int ReadOtherInfo() {
        basic_information = ReadBasicInfo();
        return basic_information.other_info_;
    }

    void WriteOtherInfo(int oth) {
        basic_information = ReadBasicInfo();
        basic_information.other_info_ = oth;
        WriteBasicInfo(basic_information);
    }

    BPlusTree() = default;

    BPlusTree(const std::string &filename) {
        Initialise(filename);
    }

    ~BPlusTree() {
        ClearBuffer();
        index_file.close();
        value_file.close();
    }

};
#endif //BPLUSTREE_H
