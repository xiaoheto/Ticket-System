#include <string>
#include <fstream>
#include <cstring>

#include "vector.h"
#include "MyChar.h"
#include "MemoryRiver.h"
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
using sjtu::vector;

template<typename K,typename T>
struct KVPair {
    K index;
    T value;

    KVPair() = default;

    KVPair(const K &other, T val) {
        index = other;
        value = val;
    }

    KVPair(const KVPair &other) {
        index = other.index;
        value = other.value;
    }

    bool operator==(const KVPair &other) const {
        return index == other.index && value == other.value;
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

template<class K,class T,int MAX = 120,int MIN = MAX / 2,int Buffer_size = 120>
class BPlusTree {
    class Node {
        bool Is_leaf;
        int size;
        KVPair<K,T> kv_pair[MAX + 2];
        int ptr[MAX + 2];
        int brother;
        friend class BPlusTree;

    public:
        Node() : kv_pair(), ptr(), brother(0) {
            for (int i = 0; i < MAX + 1; i++) {
                ptr[i] = 0;
            }
        }
    };

private:
    MemoryRiver<Node> MR;
    KVPair<K,T> pass;
    int root;

    int findPos(int l,int r,Node cur_node,KVPair<K,T>kv) {
        while(l < r) {
            int mid = (l + r) / 2;
            if (cur_node.kv_pair[mid] > kv) {
                r = mid;
            }
            else {
                l = mid + 1;
            }
        }
        return l;
    }

    bool Insert(Node cur_node, int pos, KVPair<K,T> kv) {
        if (cur_node.Is_leaf) {
            int l = findPos(0,cur_node.size,cur_node,kv);

            if (l > 0 && cur_node.kv_pair[l - 1] == kv)
                return false;

            if (cur_node.size < MAX) {
                for (int i = cur_node.size - 1; i >= l; --i) {
                    cur_node.kv_pair[i + 1] = cur_node.kv_pair[i];
                }
                cur_node.size++;
                cur_node.kv_pair[l] = kv;
                MR.update(cur_node, pos);
                return false;
            }

            for (int i = cur_node.size - 1; i >= l; --i) {
                cur_node.kv_pair[i + 1] = cur_node.kv_pair[i];
            }
            cur_node.size++;
            cur_node.kv_pair[l] = kv;
            int new_pos = MR.get_index();

            static Node newbro;
            newbro.Is_leaf = true;
            newbro.size = MIN + 1;
            newbro.brother = cur_node.brother;
            cur_node.brother = new_pos;
            for (int i = 0; i <= MIN; ++i) {
                newbro.kv_pair[i] = cur_node.kv_pair[i + MIN];
            }

            cur_node.size = MIN;
            if (root == pos) {

                static Node new_root;
                new_root.Is_leaf = false;
                new_root.size = 1;
                new_root.kv_pair[0] = cur_node.kv_pair[MIN];
                new_root.ptr[0] = pos;
                new_root.ptr[1] = new_pos;

                MR.update(cur_node, pos);
                MR.update(newbro, new_pos);
                int rootpos = MR.get_index();
                MR.write(new_root);
                root = rootpos;
                return false;
            }
            MR.update(cur_node, pos);
            MR.update(newbro, new_pos);
            pass = newbro.kv_pair[0];
            return true;
        }
        int l = findPos(0,cur_node.size,cur_node,kv);

        if (l < cur_node.size && cur_node.kv_pair[l] == kv) {
            ++l;
        }

        Node child;
        MR.read(child, cur_node.ptr[l]);

        bool state = Insert(child, cur_node.ptr[l], kv);
        if (!state)
            return false;
        if (cur_node.size < MAX) {
            for (int i = cur_node.size - 1; i >= l; --i) {
                cur_node.kv_pair[i + 1] = cur_node.kv_pair[i];
                cur_node.ptr[i + 2] = cur_node.ptr[i + 1];
            }
            ++cur_node.size;
            cur_node.kv_pair[l] = pass;
            cur_node.ptr[l + 1] = MR.get_index()-1;
            MR.update(cur_node, pos);
            return false;
        }
        for (int i = cur_node.size - 1; i >= l; --i) {
            cur_node.kv_pair[i + 1] = cur_node.kv_pair[i];
            cur_node.ptr[i + 2] = cur_node.ptr[i + 1];
        }
        ++cur_node.size;
        cur_node.kv_pair[l] = pass;
        cur_node.ptr[l + 1] = MR.get_index()-1;

        int new_pos = MR.get_index();
        pass = cur_node.kv_pair[MIN];
        static Node newbro;
        newbro.Is_leaf = false;
        newbro.size = MIN;
        for (int i = 0; i < MIN; ++i) {
            newbro.kv_pair[i] = cur_node.kv_pair[i + MIN + 1];
            newbro.ptr[i] = cur_node.ptr[i + MIN + 1];
        }
        newbro.ptr[MIN] = cur_node.ptr[cur_node.size];

        cur_node.size = MIN;
        if (root == pos) {
            static Node new_root;
            new_root.Is_leaf = false;
            new_root.size = 1;
            new_root.kv_pair[0] = pass;
            new_root.ptr[0] = pos;
            new_root.ptr[1] = new_pos;
            MR.update(cur_node, pos);
            MR.update(newbro, new_pos);
            root = MR.write(new_root);
            return false;
        }
        MR.update(cur_node, pos);
        MR.update(newbro, new_pos);
        return true;
    }

    bool Delete(Node &cur_node, int pos, const KVPair<K,T> &kv) {
        if (cur_node.Is_leaf) {
            int l = findPos(0,cur_node.size,cur_node,kv);

            --l;
            if (l < 0 || l >= cur_node.size || cur_node.kv_pair[l] != kv) {
                return false;
            }
            for (int i = l + 1; i < cur_node.size; ++i) {
                cur_node.kv_pair[i - 1] = cur_node.kv_pair[i];
            }
            --cur_node.size;
            if (pos == root) {
                MR.update(cur_node, pos);
            }
            MR.update(cur_node, pos);
            if (cur_node.size < MIN) {
                return true;
            }
            return false;
        }

        int l = findPos(0,cur_node.size,cur_node,kv);

        if (l < cur_node.size && kv == cur_node.kv_pair[l]) {
            ++l;
        }
        Node child;
        MR.read(child, cur_node.ptr[l]);
        bool state = Delete(child, cur_node.ptr[l], kv);
        if (!state)
            return false;
        if (pos == root && cur_node.size == 1) {
            static Node newbro[2];
            MR.read(newbro[0], cur_node.ptr[0]);
            MR.read(newbro[1], cur_node.ptr[1]);
            if (newbro[0].size + newbro[1].size < MAX) {
                MR.read(newbro[0], cur_node.ptr[0]);
                MR.read(newbro[1], cur_node.ptr[1]);
                if (newbro[0].Is_leaf) {
                    for (int i = 0; i < newbro[1].size; ++i) {
                        newbro[0].kv_pair[i + newbro[0].size] = newbro[1].kv_pair[i];
                    }
                    newbro[0].size += newbro[1].size;
                    newbro[0].brother = newbro[1].brother;
                    root = cur_node.ptr[0];
                    MR.update(newbro[0], cur_node.ptr[0]);
                    return false;
                }
                for (int i = 0; i < newbro[1].size; ++i) {
                    newbro[0].kv_pair[i + newbro[0].size + 1] = newbro[1].kv_pair[i];
                    newbro[0].ptr[i + newbro[0].size + 1] = newbro[1].ptr[i];
                }
                newbro[0].ptr[newbro[0].size + newbro[1].size + 1] = newbro[1].ptr[newbro[1].size];
                newbro[0].kv_pair[newbro[0].size] = cur_node.kv_pair[0];
                newbro[0].size += newbro[1].size + 1;
                root = cur_node.ptr[0];
                MR.update(newbro[0], cur_node.ptr[0]);
                return false;
            }
        }
        if (l > 0) {
            static Node newbro;
            MR.read(newbro, cur_node.ptr[l - 1]);
            if (newbro.size > MIN) {
                if (child.Is_leaf) {
                    MR.read(newbro, cur_node.ptr[l - 1]);
                    for (int i = child.size - 1; i >= 0; --i) {
                        child.kv_pair[i + 1] = child.kv_pair[i];
                    }
                    child.kv_pair[0] = newbro.kv_pair[newbro.size - 1];
                    ++child.size;
                    --newbro.size;
                    cur_node.kv_pair[l - 1] = child.kv_pair[0];
                    MR.update(cur_node, pos);
                    MR.update(newbro, cur_node.ptr[l - 1]);
                    MR.update(child, cur_node.ptr[l]);
                    return false;
                }
                MR.read(newbro, cur_node.ptr[l - 1]);
                for (int i = child.size; i >= 1; --i) {
                    child.kv_pair[i] = child.kv_pair[i - 1];
                    child.ptr[i + 1] = child.ptr[i];
                }
                child.ptr[1] = child.ptr[0];
                ++child.size;
                child.kv_pair[0] = cur_node.kv_pair[l - 1];
                child.ptr[0] = newbro.ptr[newbro.size];
                cur_node.kv_pair[l - 1] = newbro.kv_pair[newbro.size - 1];
                --newbro.size;
                MR.update(cur_node, pos);
                MR.update(newbro, cur_node.ptr[l - 1]);
                MR.update(child, cur_node.ptr[l]);
                return false;
            }
            if (child.Is_leaf) {
                MR.read(newbro, cur_node.ptr[l - 1]);
                for (int i = 0; i < child.size; ++i) {
                    newbro.kv_pair[i + newbro.size] = child.kv_pair[i];
                }
                newbro.size += child.size;
                newbro.brother = child.brother;
                for (int i = l; i < cur_node.size; ++i) {
                    cur_node.kv_pair[i - 1] = cur_node.kv_pair[i];
                    cur_node.ptr[i] = cur_node.ptr[i + 1];
                }
                --cur_node.size;
                newbro.brother = child.brother;
                MR.update(cur_node, pos);
                MR.update(newbro, cur_node.ptr[l - 1]);
                if (cur_node.size < MIN)
                    return true;
                return false;
            }
            MR.read(newbro, cur_node.ptr[l - 1]);
            for (int i = 0; i < child.size; ++i) {
                newbro.kv_pair[i + newbro.size + 1] = child.kv_pair[i];
                newbro.ptr[i + newbro.size + 1] = child.ptr[i];
            }
            newbro.ptr[newbro.size + child.size + 1] = child.ptr[child.size];
            newbro.kv_pair[newbro.size] = cur_node.kv_pair[l - 1];
            newbro.size += child.size + 1;
            for (int i = l - 1; i < cur_node.size - 1; ++i) {
                cur_node.kv_pair[i] = cur_node.kv_pair[i + 1];
                cur_node.ptr[i + 1] = cur_node.ptr[i + 2];
            }
            --cur_node.size;
            MR.update(cur_node, pos);
            MR.update(newbro, cur_node.ptr[l - 1]);
            if (cur_node.size < MIN)
                return true;
            return false;
        }
        else if (l < cur_node.size) {
            static Node newbro;
            MR.read(newbro, cur_node.ptr[l + 1]);
            if (newbro.size > MIN) {
                if (child.Is_leaf) {
                    MR.read(newbro, cur_node.ptr[l + 1]);
                    child.kv_pair[child.size] = newbro.kv_pair[0];
                    ++child.size;
                    for (int i = 0; i < newbro.size - 1; ++i) {
                        newbro.kv_pair[i] = newbro.kv_pair[i + 1];
                    }
                    --newbro.size;
                    cur_node.kv_pair[l] = newbro.kv_pair[0];
                    MR.update(cur_node, pos);
                    MR.update(child, cur_node.ptr[l]);
                    MR.update(newbro, cur_node.ptr[l + 1]);
                    return false;
                }
                MR.read(newbro, cur_node.ptr[l + 1]);
                child.kv_pair[child.size] = cur_node.kv_pair[l];
                child.ptr[child.size + 1] = newbro.ptr[0];
                ++child.size;
                cur_node.kv_pair[l] = newbro.kv_pair[0];
                for (int i = 0; i < newbro.size - 1; ++i) {
                    newbro.kv_pair[i] = newbro.kv_pair[i + 1];
                    newbro.ptr[i] = newbro.ptr[i + 1];
                }
                newbro.ptr[newbro.size - 1] = newbro.ptr[newbro.size];
                --newbro.size;
                MR.update(cur_node, pos);
                MR.update(child, cur_node.ptr[l]);
                MR.update(newbro, cur_node.ptr[l + 1]);
                return false;
            }
            if (child.Is_leaf) {
                MR.read(newbro, cur_node.ptr[l + 1]);
                for (int i = 0; i < newbro.size; ++i) {
                    child.kv_pair[i + child.size] = newbro.kv_pair[i];
                }
                child.size += newbro.size;
                child.brother = newbro.brother;
                for (int i = l; i < cur_node.size - 1; ++i) {
                    cur_node.kv_pair[i] = cur_node.kv_pair[i + 1];
                    cur_node.ptr[i + 1] = cur_node.ptr[i + 2];
                }
                --cur_node.size;
                child.brother = newbro.brother;
                MR.update(cur_node, pos);
                MR.update(child, cur_node.ptr[l]);
                if (cur_node.size < MIN)
                    return true;
                return false;
            }
            MR.read(newbro, cur_node.ptr[l + 1]);
            for (int i = 0; i < newbro.size; ++i) {
                child.kv_pair[i + child.size + 1] = newbro.kv_pair[i];
                child.ptr[i + child.size + 1] = newbro.ptr[i];
            }
            child.ptr[child.size + newbro.size + 1] = newbro.ptr[newbro.size];
            child.kv_pair[child.size] = cur_node.kv_pair[l];
            child.size += newbro.size + 1;
            for (int i = l; i < cur_node.size - 1; ++i) {
                cur_node.kv_pair[i] = cur_node.kv_pair[i + 1];
                cur_node.ptr[i + 1] = cur_node.ptr[i + 2];
            }
            --cur_node.size;
            MR.update(cur_node,pos );
            MR.update(child, cur_node.ptr[l]);
            if (cur_node.size < MIN)
                return true;
            return false;
        }
        else {
            throw;
        }
    }

public:
    BPlusTree() {
        MR.initialise("BPlusTree.dat");
        MR.get_info(root, 1);
    }

    ~BPlusTree() {
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
            x.kv_pair[0] = kv;
            x.size = 1;
            x.Is_leaf = true;
            x.brother = -1;
            root = MR.write(x);
        } else {
            Node cur_node;
            MR.read(cur_node, root);
            Insert(cur_node, root, kv);
        }
    }

    void erase(K index, T value) {
        if (root == -1) return;
        KVPair<K,T> kv(index, value);
        Node cur_node;
        MR.read(cur_node, root);
        Delete(cur_node, root, kv);
    }

    vector<T> query(K index) {
        vector<T> ans;
        ans.clear();
        if(root==-1){
            return ans;
        }
        Node cur_node;
        MR.read(cur_node, root);
        while (!cur_node.Is_leaf) {
            int i=0;
            for (; i < cur_node.size; i++) {
                if (index < cur_node.kv_pair[i].index && (i == -1 || index >= cur_node.kv_pair[i - 1].index)){
                    break;
                }
            }
            MR.read(cur_node, cur_node.ptr[i]);
        }
        for (int i = 0; i <= cur_node.size; i++) {
            if (i == cur_node.size) {
                if (cur_node.brother == -1) {
                    break;
                }
                MR.read(cur_node, cur_node.brother);
                i = -1;
                continue;
            }
            if (index < cur_node.kv_pair[i].index) {
                break;
            }
            if (index == cur_node.kv_pair[i].index) {
                ans.push_back(cur_node.kv_pair[i].value);
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
