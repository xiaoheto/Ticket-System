#include <string>
#include <fstream>
#include <cstring>

#include "vector.hpp"
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

template<typename T,int length>
struct KVPair {
    MyChar<length> index;
    T value;

    KVPair() = default;

    KVPair(MyChar<length> &other, T val) {
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

template<class T,int MAX = 120,int MIN = MAX / 2,int index_length = 65>
class BPlusTree {
    class Node {
        bool is_leaf;
        int size;
        KVPair<T,index_length> kv_pair[MAX + 2];
        int sib;
        int son[MAX + 2];
        friend class BPlusTree;

    public:
        Node() : kv_pair(), son(), sib(0) {
            for (int i = 0; i < MAX + 1; i++) {
                son[i] = 0;
            }
        }
    };

    int root;
    MemoryRiver<Node> base_file;
    KVPair<T,index_length> pass;

    //find the first pos that is larger than kv
    int findPos(int l, int r, Node node, KVPair<T,index_length> kv) {
        while (l < r) {
            int mid = (l + r) / 2;
            if (node.kv_pair[mid] > kv) {
                r = mid;
            } else {
                l = mid + 1;
            }
        }
        return l;
    }

    bool Insert(Node cur_node, int pos, KVPair<T,index_length> kv) {
        if (cur_node.is_leaf) {
            int l = findPos(0, cur_node.size, cur_node, kv);

            if (l > 0 && l < cur_node.size && cur_node.kv_pair[l - 1] == kv) {
                return false;
            }

            if (cur_node.size < MAX) {
                for (int i = cur_node.size - 1; i >= l; --i) {
                    cur_node.kv_pair[i + 1] = cur_node.kv_pair[i];
                }
                cur_node.size++;
                cur_node.kv_pair[l] = kv;
                base_file.update(cur_node, pos);
                return false;
            }

            for (int i = cur_node.size - 1; i >= l; --i) {
                cur_node.kv_pair[i + 1] = cur_node.kv_pair[i];
            }
            cur_node.size++;
            cur_node.kv_pair[l] = kv;
            int newpos = base_file.get_index();

            static Node newbro;
            newbro.is_leaf = true;
            newbro.size = MIN + 1;
            newbro.sib = cur_node.sib;
            cur_node.sib = newpos;
            for (int i = 0; i <= MIN; ++i) {
                newbro.kv_pair[i] = cur_node.kv_pair[i + MIN];
            }

            cur_node.size = MIN;
            if (root == pos) {

                static Node newroot;
                newroot.is_leaf = false;
                newroot.size = 1;
                newroot.kv_pair[0] = cur_node.kv_pair[MIN];
                newroot.son[0] = pos;
                newroot.son[1] = newpos;

                base_file.update(cur_node, pos);
                base_file.update(newbro, newpos);
                int rootpos = base_file.get_index();
                base_file.write(newroot);
                root = rootpos;
                return false;
            }
            base_file.update(cur_node, pos);
            base_file.update(newbro, newpos);
            pass = newbro.kv_pair[0];
            return true;
        }

        int l = findPos(0,cur_node.size,cur_node,kv);
        if (l < cur_node.size && cur_node.kv_pair[l] == kv) {
            ++l;
        }

        Node child;
        base_file.read(child, cur_node.son[l]);

        bool state = Insert(child, cur_node.son[l], kv);
        if (!state) {
            return false;
        }
        if (cur_node.size < MAX) {
            for (int i = cur_node.size - 1; i >= l; --i) {
                cur_node.kv_pair[i + 1] = cur_node.kv_pair[i];
                cur_node.son[i + 2] = cur_node.son[i + 1];
            }
            cur_node.size++;
            cur_node.kv_pair[l] = pass;
            cur_node.son[l + 1] = base_file.get_index()-1;
            base_file.update(cur_node, pos);
            return false;
        }
        for (int i = cur_node.size - 1; i >= l; --i) {
            cur_node.kv_pair[i + 1] = cur_node.kv_pair[i];
            cur_node.son[i + 2] = cur_node.son[i + 1];
        }
        ++cur_node.size;
        cur_node.kv_pair[l] = pass;
        cur_node.son[l + 1] = base_file.get_index()-1;

        int newpos = base_file.get_index();
        pass = cur_node.kv_pair[MIN];
        static Node newbro;
        newbro.is_leaf = false;
        newbro.size = MIN;
        for (int i = 0; i < MIN; ++i) {
            newbro.kv_pair[i] = cur_node.kv_pair[i + MIN + 1];
            newbro.son[i] = cur_node.son[i + MIN + 1];
        }
        newbro.son[MIN] = cur_node.son[cur_node.size];

        cur_node.size = MIN;
        if (root == pos) {
            static Node newroot;
            newroot.is_leaf = false;
            newroot.size = 1;
            newroot.kv_pair[0] = pass;
            newroot.son[0] = pos;
            newroot.son[1] = newpos;
            base_file.update(cur_node, pos);
            base_file.update(newbro, newpos);
            root = base_file.write(newroot);
            return false;
        }
        base_file.update(cur_node, pos);
        base_file.update(newbro, newpos);
        return true;
    }

    bool Delete(Node &cur_node, int pos, const KVPair<T,index_length> &kv) {
        if (cur_node.is_leaf) {
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
                base_file.update(cur_node, pos);
            }
            base_file.update(cur_node, pos);
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
        base_file.read(child, cur_node.son[l]);
        bool state = Delete(child, cur_node.son[l], kv);
        if (!state)
            return false;
        if (pos == root && cur_node.size == 1) {
            static Node newbro[2];
            base_file.read(newbro[0], cur_node.son[0]);
            base_file.read(newbro[1], cur_node.son[1]);
            if (newbro[0].size + newbro[1].size < MAX) {
                base_file.read(newbro[0], cur_node.son[0]);
                base_file.read(newbro[1], cur_node.son[1]);
                if (newbro[0].is_leaf) {
                    for (int i = 0; i < newbro[1].size; ++i) {
                        newbro[0].kv_pair[i + newbro[0].size] = newbro[1].kv_pair[i];
                    }
                    newbro[0].size += newbro[1].size;
                    newbro[0].sib = newbro[1].sib;
                    root = cur_node.son[0];
                    base_file.update(newbro[0], cur_node.son[0]);
                    return false;
                }
                for (int i = 0; i < newbro[1].size; ++i) {
                    newbro[0].kv_pair[i + newbro[0].size + 1] = newbro[1].kv_pair[i];
                    newbro[0].son[i + newbro[0].size + 1] = newbro[1].son[i];
                }
                newbro[0].son[newbro[0].size + newbro[1].size + 1] = newbro[1].son[newbro[1].size];
                newbro[0].kv_pair[newbro[0].size] = cur_node.kv_pair[0];
                newbro[0].size += newbro[1].size + 1;
                root = cur_node.son[0];
                base_file.update(newbro[0], cur_node.son[0]);
                return false;
            }
        }
        if (l > 0) {
            static Node newbro;
            base_file.read(newbro, cur_node.son[l - 1]);
            if (newbro.size > MIN) {
                if (child.is_leaf) {
                    base_file.read(newbro, cur_node.son[l - 1]);
                    for (int i = child.size - 1; i >= 0; --i) {
                        child.kv_pair[i + 1] = child.kv_pair[i];
                    }
                    child.kv_pair[0] = newbro.kv_pair[newbro.size - 1];
                    ++child.size;
                    --newbro.size;
                    cur_node.kv_pair[l - 1] = child.kv_pair[0];
                    base_file.update(cur_node, pos);
                    base_file.update(newbro, cur_node.son[l - 1]);
                    base_file.update(child, cur_node.son[l]);
                    return false;
                }
                base_file.read(newbro, cur_node.son[l - 1]);
                for (int i = child.size; i >= 1; --i) {
                    child.kv_pair[i] = child.kv_pair[i - 1];
                    child.son[i + 1] = child.son[i];
                }
                child.son[1] = child.son[0];
                ++child.size;
                child.kv_pair[0] = cur_node.kv_pair[l - 1];
                child.son[0] = newbro.son[newbro.size];
                cur_node.kv_pair[l - 1] = newbro.kv_pair[newbro.size - 1];
                --newbro.size;
                base_file.update(cur_node, pos);
                base_file.update(newbro, cur_node.son[l - 1]);
                base_file.update(child, cur_node.son[l]);
                return false;
            }
            if (child.is_leaf) {
                base_file.read(newbro, cur_node.son[l - 1]);
                for (int i = 0; i < child.size; ++i) {
                    newbro.kv_pair[i + newbro.size] = child.kv_pair[i];
                }
                newbro.size += child.size;
                newbro.sib = child.sib;
                for (int i = l; i < cur_node.size; ++i) {
                    cur_node.kv_pair[i - 1] = cur_node.kv_pair[i];
                    cur_node.son[i] = cur_node.son[i + 1];
                }
                --cur_node.size;
                newbro.sib = child.sib;
                base_file.update(cur_node, pos);
                base_file.update(newbro, cur_node.son[l - 1]);
                if (cur_node.size < MIN)
                    return true;
                return false;
            }
            base_file.read(newbro, cur_node.son[l - 1]);
            for (int i = 0; i < child.size; ++i) {
                newbro.kv_pair[i + newbro.size + 1] = child.kv_pair[i];
                newbro.son[i + newbro.size + 1] = child.son[i];
            }
            newbro.son[newbro.size + child.size + 1] = child.son[child.size];
            newbro.kv_pair[newbro.size] = cur_node.kv_pair[l - 1];
            newbro.size += child.size + 1;
            for (int i = l - 1; i < cur_node.size - 1; ++i) {
                cur_node.kv_pair[i] = cur_node.kv_pair[i + 1];
                cur_node.son[i + 1] = cur_node.son[i + 2];
            }
            --cur_node.size;
            base_file.update(cur_node, pos);
            base_file.update(newbro, cur_node.son[l - 1]);
            if (cur_node.size < MIN)
                return true;
            return false;
        } else if (l < cur_node.size) {
            static Node newbro;
            base_file.read(newbro, cur_node.son[l + 1]);
            if (newbro.size > MIN) {
                if (child.is_leaf) {
                    base_file.read(newbro, cur_node.son[l + 1]);
                    child.kv_pair[child.size] = newbro.kv_pair[0];
                    ++child.size;
                    for (int i = 0; i < newbro.size - 1; ++i) {
                        newbro.kv_pair[i] = newbro.kv_pair[i + 1];
                    }
                    --newbro.size;
                    cur_node.kv_pair[l] = newbro.kv_pair[0];
                    base_file.update(cur_node, pos);
                    base_file.update(child, cur_node.son[l]);
                    base_file.update(newbro, cur_node.son[l + 1]);
                    return false;
                }
                base_file.read(newbro, cur_node.son[l + 1]);
                child.kv_pair[child.size] = cur_node.kv_pair[l];
                child.son[child.size + 1] = newbro.son[0];
                ++child.size;
                cur_node.kv_pair[l] = newbro.kv_pair[0];
                for (int i = 0; i < newbro.size - 1; ++i) {
                    newbro.kv_pair[i] = newbro.kv_pair[i + 1];
                    newbro.son[i] = newbro.son[i + 1];
                }
                newbro.son[newbro.size - 1] = newbro.son[newbro.size];
                --newbro.size;
                base_file.update(cur_node, pos);
                base_file.update(child, cur_node.son[l]);
                base_file.update(newbro, cur_node.son[l + 1]);
                return false;
            }
            if (child.is_leaf) {
                base_file.read(newbro, cur_node.son[l + 1]);
                for (int i = 0; i < newbro.size; ++i) {
                    child.kv_pair[i + child.size] = newbro.kv_pair[i];
                }
                child.size += newbro.size;
                child.sib = newbro.sib;
                for (int i = l; i < cur_node.size - 1; ++i) {
                    cur_node.kv_pair[i] = cur_node.kv_pair[i + 1];
                    cur_node.son[i + 1] = cur_node.son[i + 2];
                }
                --cur_node.size;
                child.sib = newbro.sib;
                base_file.update(cur_node, pos);
                base_file.update(child, cur_node.son[l]);
                if (cur_node.size < MIN)
                    return true;
                return false;
            }
            base_file.read(newbro, cur_node.son[l + 1]);
            for (int i = 0; i < newbro.size; ++i) {
                child.kv_pair[i + child.size + 1] = newbro.kv_pair[i];
                child.son[i + child.size + 1] = newbro.son[i];
            }
            child.son[child.size + newbro.size + 1] = newbro.son[newbro.size];
            child.kv_pair[child.size] = cur_node.kv_pair[l];
            child.size += newbro.size + 1;
            for (int i = l; i < cur_node.size - 1; ++i) {
                cur_node.kv_pair[i] = cur_node.kv_pair[i + 1];
                cur_node.son[i + 1] = cur_node.son[i + 2];
            }
            --cur_node.size;
            base_file.update(cur_node,pos );
            base_file.update(child, cur_node.son[l]);
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
        base_file.initialise("BPlusTree");
        base_file.get_info(root, 1);
    }

    ~BPlusTree() {
        base_file.write_info(root, 1);
        base_file.end();
    };

    bool empty(){
        return root == -1;
    }

    void insert(MyChar<index_length> index, T value) {
        KVPair<T,index_length> kv(index, value);
        if (root == -1) {
            Node x;
            x.kv_pair[0] = kv;
            x.size = 1;
            x.is_leaf = true;
            x.sib = -1;
            root = base_file.write(x);
        } else {
            Node cur_node;
            base_file.read(cur_node, root);
            Insert(cur_node, root, kv);
        }
    }

    void erase(MyChar<index_length>index, T value) {
        if (root == -1) {
            return;
        }
        KVPair<T,index_length> kv(index, value);
        Node cur_node;
        base_file.read(cur_node, root);
        Delete(cur_node, root, kv);
    }

    vector<T> query(MyChar<index_length>index) {
        vector<T> ans;
        ans.clear();
        if(root == -1){
            return ans;
        }
        Node cur_node;
        base_file.read(cur_node, root);
        while (!cur_node.is_leaf) {
            int i = 0;
            for (; i < cur_node.size; i++) {
                if (std::strcmp(index.c_str(), cur_node.kv_pair[i].index.c_str()) <= 0 &&
    (i - 1 == -1 || std::strcmp(index.c_str(), cur_node.kv_pair[i - 1].index.c_str()) >= 0)) {
                    break;
                }
            }
            base_file.read(cur_node, cur_node.son[i]);
        }
        for (int i = 0; i <= cur_node.size; i++) {
            if (i == cur_node.size) {
                if (cur_node.sib == -1) {
                    break;
                }
                base_file.read(cur_node, cur_node.sib);
                i = -1;
                continue;
            }
            if (std::strcmp(index.c_str(), cur_node.kv_pair[i].index.c_str()) < 0) {
                break;
            }
            if (std::strcmp(index.c_str(), cur_node.kv_pair[i].index.c_str()) == 0) {
                ans.push_back(cur_node.kv_pair[i].value);
                continue;
            }
        }
        return ans;
    }

    void end(){
        base_file.end();
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
