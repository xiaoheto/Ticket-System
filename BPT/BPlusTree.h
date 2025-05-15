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

#include "vector.h";
#include "MyChar.h"
#include "MemoryRiver.h"
using sjtu::vector;

template<class K,class T>
struct KVPair {
    K index;
    T value;

    KVPair() = default;

    KVPair(K index, T value) {
        this->index = index;
        this->value = value;
    }

    KVPair(const KVPair &other) {
        index = other.index;
        value = other.value;
    }

    bool operator==(const KVPair &other) const {
        return index ==  other.index && value == other.value;
    }

    bool operator!=(const KVPair &other) const {
        return !(*this == other);
    }

    bool operator<(const KVPair &other) const {
        if (index != other.index) {
            return index < other.index;
        }
        return value < other.value;
    }

    bool operator<=(const KVPair &other) const {
        return *this < other || *this == other;
    }

    bool operator>(const KVPair &other) const {
        if (index != other.index) {
            return index > other.index;
        }
        return value > other.value;
    }

    bool operator>=(const KVPair &other) const {
        return *this > other || *this == other;
    }
};

template<typename K,typename T,int MAX = 102,int MIN = MAX / 2>
class BPTree {
    class Node {
        bool Is_leaf;
        int size;
        KVPair<K,T> key_value[MAX + 2];
        int ptr[MAX + 2];
        int brother;
        friend class BPTree;

    public:
        Node() : key_value(), ptr(), brother(0) {
            for (int i = 0; i < MAX + 1; i++) {
                ptr[i] = 0;
            }
        }
    };
public:
    int root;
private:
    MemoryRiver<Node> MR;
    KVPair<K,T> pass;

    bool insertInternal(Node cursor, int pos, KVPair<K,T> kv) {
        if (cursor.Is_leaf) {
            int l = 0, r = cursor.size;
            while (l < r) {
                int mid = (l + r) >> 1;
                if (cursor.key_value[mid] > kv) {
                    r = mid;
                } else {
                    l = mid + 1;
                }
            }

            if (l > 0 && cursor.key_value[l - 1] == kv)
                return false;

            if (cursor.size < MAX) {
                for (int i = cursor.size - 1; i >= l; --i) {
                    cursor.key_value[i + 1] = cursor.key_value[i];
                }
                cursor.size++;
                cursor.key_value[l] = kv;
                MR.update(cursor, pos);
                return false;
            }

            for (int i = cursor.size - 1; i >= l; --i) {
                cursor.key_value[i + 1] = cursor.key_value[i];
            }
            cursor.size++;
            cursor.key_value[l] = kv;
            int newpos = MR.getindex();

            static Node newbro;
            newbro.Is_leaf = true;
            newbro.size = MIN + 1;
            newbro.brother = cursor.brother;
            cursor.brother = newpos;
            for (int i = 0; i <= MIN; ++i) {
                newbro.key_value[i] = cursor.key_value[i + MIN];
            }

            cursor.size = MIN;
            if (root == pos) {

                static Node newroot;
                newroot.Is_leaf = false;
                newroot.size = 1;
                newroot.key_value[0] = cursor.key_value[MIN];
                newroot.ptr[0] = pos;
                newroot.ptr[1] = newpos;

                MR.update(cursor, pos);
                MR.update(newbro, newpos);
                int rootpos = MR.getindex();
                MR.write(newroot);
                root = rootpos;
                return false;
            }
            MR.update(cursor, pos);
            MR.update(newbro, newpos);
            pass = newbro.key_value[0];
            return true;
        }

        int l = 0, r = cursor.size;
        while (l < r) {
            int mid = (l + r) >> 1;
            if (cursor.key_value[mid] >= kv) {
                r = mid;
            } else {
                l = mid + 1;
            }
        }
        if (l < cursor.size && cursor.key_value[l] == kv) {
            ++l;
        }

        Node child;
        MR.read(child, cursor.ptr[l]);

        bool state = insertInternal(child, cursor.ptr[l], kv);
        if (!state)
            return false;
        if (cursor.size < MAX) {
            for (int i = cursor.size - 1; i >= l; --i) {
                cursor.key_value[i + 1] = cursor.key_value[i];
                cursor.ptr[i + 2] = cursor.ptr[i + 1];
            }
            cursor.size++;
            cursor.key_value[l] = pass;
            cursor.ptr[l + 1] = MR.getindex()-1;
            MR.update(cursor, pos);
            return false;
        }
        for (int i = cursor.size - 1; i >= l; --i) {
            cursor.key_value[i + 1] = cursor.key_value[i];
            cursor.ptr[i + 2] = cursor.ptr[i + 1];
        }
        ++cursor.size;
        cursor.key_value[l] = pass;
        cursor.ptr[l + 1] = MR.getindex()-1;

        int newpos = MR.getindex();
        pass = cursor.key_value[MIN];
        static Node newbro;
        newbro.Is_leaf = false;
        newbro.size = MIN;
        for (int i = 0; i < MIN; ++i) {
            newbro.key_value[i] = cursor.key_value[i + MIN + 1];
            newbro.ptr[i] = cursor.ptr[i + MIN + 1];
        }
        newbro.ptr[MIN] = cursor.ptr[cursor.size];

        cursor.size = MIN;
        if (root == pos) {
            static Node newroot;
            newroot.Is_leaf = false;
            newroot.size = 1;
            newroot.key_value[0] = pass;
            newroot.ptr[0] = pos;
            newroot.ptr[1] = newpos;
            MR.update(cursor, pos);
            MR.update(newbro, newpos);
            root = MR.write(newroot);
            return false;
        }
        MR.update(cursor, pos);
        MR.update(newbro, newpos);
        return true;
    }

    bool deleteInternal(Node &cursor, int pos, const KVPair<K,T> &kv) {
        if (cursor.Is_leaf) {
            int l = 0, r = cursor.size;
            while (l < r) {
                int mid = (l + r) >> 1;
                if (cursor.key_value[mid] > kv) {
                    r = mid;
                } else {
                    l = mid + 1;
                }
            }
            --l;
            if (l < 0 || l >= cursor.size || cursor.key_value[l] != kv) {
                return false;
            }
            for (int i = l + 1; i < cursor.size; ++i) {
                cursor.key_value[i - 1] = cursor.key_value[i];
            }
            --cursor.size;
            if (pos == root) {
                MR.update(cursor, pos);
            }
            MR.update(cursor, pos);
            if (cursor.size < MIN) {
                return true;
            }
            return false;
        }

        int l = 0, r = cursor.size;
        while (l < r) {
            int mid = (l + r) >> 1;
            if (cursor.key_value[mid] >= kv) {
                r = mid;
            } else {
                l = mid + 1;
            }
        }
        if (l < cursor.size && kv == cursor.key_value[l]) {
            ++l;
        }
        Node child;
        MR.read(child, cursor.ptr[l]);
        bool state = deleteInternal(child, cursor.ptr[l], kv);
        if (!state)
            return false;
        if (pos == root && cursor.size == 1) {
            static Node newbro[2];
            MR.read(newbro[0], cursor.ptr[0]);
            MR.read(newbro[1], cursor.ptr[1]);
            if (newbro[0].size + newbro[1].size < MAX) {
                MR.read(newbro[0], cursor.ptr[0]);
                MR.read(newbro[1], cursor.ptr[1]);
                if (newbro[0].Is_leaf) {
                    for (int i = 0; i < newbro[1].size; ++i) {
                        newbro[0].key_value[i + newbro[0].size] = newbro[1].key_value[i];
                    }
                    newbro[0].size += newbro[1].size;
                    newbro[0].brother = newbro[1].brother;
                    root = cursor.ptr[0];
                    MR.update(newbro[0], cursor.ptr[0]);
                    return false;
                }
                for (int i = 0; i < newbro[1].size; ++i) {
                    newbro[0].key_value[i + newbro[0].size + 1] = newbro[1].key_value[i];
                    newbro[0].ptr[i + newbro[0].size + 1] = newbro[1].ptr[i];
                }
                newbro[0].ptr[newbro[0].size + newbro[1].size + 1] = newbro[1].ptr[newbro[1].size];
                newbro[0].key_value[newbro[0].size] = cursor.key_value[0];
                newbro[0].size += newbro[1].size + 1;
                root = cursor.ptr[0];
                MR.update(newbro[0], cursor.ptr[0]);
                return false;
            }
        }
        if (l > 0) {
            static Node newbro;
            MR.read(newbro, cursor.ptr[l - 1]);
            if (newbro.size > MIN) {
                if (child.Is_leaf) {
                    MR.read(newbro, cursor.ptr[l - 1]);
                    for (int i = child.size - 1; i >= 0; --i) {
                        child.key_value[i + 1] = child.key_value[i];
                    }
                    child.key_value[0] = newbro.key_value[newbro.size - 1];
                    ++child.size;
                    --newbro.size;
                    cursor.key_value[l - 1] = child.key_value[0];
                    MR.update(cursor, pos);
                    MR.update(newbro, cursor.ptr[l - 1]);
                    MR.update(child, cursor.ptr[l]);
                    return false;
                }
                MR.read(newbro, cursor.ptr[l - 1]);
                for (int i = child.size; i >= 1; --i) {
                    child.key_value[i] = child.key_value[i - 1];
                    child.ptr[i + 1] = child.ptr[i];
                }
                child.ptr[1] = child.ptr[0];
                ++child.size;
                child.key_value[0] = cursor.key_value[l - 1];
                child.ptr[0] = newbro.ptr[newbro.size];
                cursor.key_value[l - 1] = newbro.key_value[newbro.size - 1];
                --newbro.size;
                MR.update(cursor, pos);
                MR.update(newbro, cursor.ptr[l - 1]);
                MR.update(child, cursor.ptr[l]);
                return false;
            }
            if (child.Is_leaf) {
                MR.read(newbro, cursor.ptr[l - 1]);
                for (int i = 0; i < child.size; ++i) {
                    newbro.key_value[i + newbro.size] = child.key_value[i];
                }
                newbro.size += child.size;
                newbro.brother = child.brother;
                for (int i = l; i < cursor.size; ++i) {
                    cursor.key_value[i - 1] = cursor.key_value[i];
                    cursor.ptr[i] = cursor.ptr[i + 1];
                }
                --cursor.size;
                newbro.brother = child.brother;
                MR.update(cursor, pos);
                MR.update(newbro, cursor.ptr[l - 1]);
                if (cursor.size < MIN)
                    return true;
                return false;
            }
            MR.read(newbro, cursor.ptr[l - 1]);
            for (int i = 0; i < child.size; ++i) {
                newbro.key_value[i + newbro.size + 1] = child.key_value[i];
                newbro.ptr[i + newbro.size + 1] = child.ptr[i];
            }
            newbro.ptr[newbro.size + child.size + 1] = child.ptr[child.size];
            newbro.key_value[newbro.size] = cursor.key_value[l - 1];
            newbro.size += child.size + 1;
            for (int i = l - 1; i < cursor.size - 1; ++i) {
                cursor.key_value[i] = cursor.key_value[i + 1];
                cursor.ptr[i + 1] = cursor.ptr[i + 2];
            }
            --cursor.size;
            MR.update(cursor, pos);
            MR.update(newbro, cursor.ptr[l - 1]);
            if (cursor.size < MIN)
                return true;
            return false;
        } else if (l < cursor.size) {
            static Node newbro;
            MR.read(newbro, cursor.ptr[l + 1]);
            if (newbro.size > MIN) {
                if (child.Is_leaf) {
                    MR.read(newbro, cursor.ptr[l + 1]);
                    child.key_value[child.size] = newbro.key_value[0];
                    ++child.size;
                    for (int i = 0; i < newbro.size - 1; ++i) {
                        newbro.key_value[i] = newbro.key_value[i + 1];
                    }
                    --newbro.size;
                    cursor.key_value[l] = newbro.key_value[0];
                    MR.update(cursor, pos);
                    MR.update(child, cursor.ptr[l]);
                    MR.update(newbro, cursor.ptr[l + 1]);
                    return false;
                }
                MR.read(newbro, cursor.ptr[l + 1]);
                child.key_value[child.size] = cursor.key_value[l];
                child.ptr[child.size + 1] = newbro.ptr[0];
                ++child.size;
                cursor.key_value[l] = newbro.key_value[0];
                for (int i = 0; i < newbro.size - 1; ++i) {
                    newbro.key_value[i] = newbro.key_value[i + 1];
                    newbro.ptr[i] = newbro.ptr[i + 1];
                }
                newbro.ptr[newbro.size - 1] = newbro.ptr[newbro.size];
                --newbro.size;
                MR.update(cursor, pos);
                MR.update(child, cursor.ptr[l]);
                MR.update(newbro, cursor.ptr[l + 1]);
                return false;
            }
            if (child.Is_leaf) {
                MR.read(newbro, cursor.ptr[l + 1]);
                for (int i = 0; i < newbro.size; ++i) {
                    child.key_value[i + child.size] = newbro.key_value[i];
                }
                child.size += newbro.size;
                child.brother = newbro.brother;
                for (int i = l; i < cursor.size - 1; ++i) {
                    cursor.key_value[i] = cursor.key_value[i + 1];
                    cursor.ptr[i + 1] = cursor.ptr[i + 2];
                }
                --cursor.size;
                child.brother = newbro.brother;
                MR.update(cursor, pos);
                MR.update(child, cursor.ptr[l]);
                if (cursor.size < MIN)
                    return true;
                return false;
            }
            MR.read(newbro, cursor.ptr[l + 1]);
            for (int i = 0; i < newbro.size; ++i) {
                child.key_value[i + child.size + 1] = newbro.key_value[i];
                child.ptr[i + child.size + 1] = newbro.ptr[i];
            }
            child.ptr[child.size + newbro.size + 1] = newbro.ptr[newbro.size];
            child.key_value[child.size] = cursor.key_value[l];
            child.size += newbro.size + 1;
            for (int i = l; i < cursor.size - 1; ++i) {
                cursor.key_value[i] = cursor.key_value[i + 1];
                cursor.ptr[i + 1] = cursor.ptr[i + 2];
            }
            --cursor.size;
            MR.update(cursor,pos );
            MR.update(child, cursor.ptr[l]);
            if (cursor.size < MIN)
                return true;
            return false;
        }
        else {
            throw;
        }
    }

public:
    BPTree() {
        MR.initialise("BPTree.dat");
        MR.get_info(root, 1);
    }

    ~BPTree() {
        MR.write_info(root, 1);
        MR.end();
    };
    bool empty(){
        return root==-1;
    }

    void insert(K index, T value) {
        KVPair<K,T> kv(index, value);
        if (root == -1) {
            Node x;
            x.key_value[0] = kv;
            x.size = 1;
            x.Is_leaf = true;
            x.brother = -1;
            root = MR.write(x);
        } else {
            Node cursor;
            MR.read(cursor, root);
            insertInternal(cursor, root, kv);
        }
    }

    void erase(K index, T value) {
        if (root == -1) return;
        KVPair<K,T> kv(index, value);
        Node cursor;
        MR.read(cursor, root);
        deleteInternal(cursor, root, kv);
    }

    vector<T> query(K index) {
        vector<T> ans;
        ans.clear();
        if(root==-1){
            return ans;
        }
        Node cursor;
        MR.read(cursor, root);
        while (!cursor.Is_leaf) {
            int i=0;
            for (; i < cursor.size; i++) {
                if ((index <= cursor.key_value[i].index) && (i - 1 == -1 || index >= cursor.key_value[i - 1].index)) {
                    break;
                }
            }
            MR.read(cursor, cursor.ptr[i]);
        }
        for (int i = 0; i <= cursor.size; i++) {
            if (i == cursor.size) {
                if (cursor.brother == -1) {
                    break;
                }
                MR.read(cursor, cursor.brother);
                i = -1;
                continue;
            }
            if (index < cursor.key_value[i].index) {
                break;
            }
            if (index == cursor.key_value[i].index) {
                ans.push_back(cursor.key_value[i].value);
                continue;
            }
        }
        return ans;
    }
    void end(){
        MR.end();
    }
};

void quick_sort(vector<int> &arr, int left, int right) {
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
    if (left < j) quick_sort(arr, left, j);
    if (i < right) quick_sort(arr, i, right);
}
