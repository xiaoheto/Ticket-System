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

template<class K,class T,int MAX = 120,int MIN = MAX / 2>
class BPTree {
    class Node {
        bool Is_leaf;
        int size;
        KVPair<K,T> kv_pair[MAX + 2];
        int ptr[MAX + 2];
        int brother;
        friend class BPTree;

    public:
        Node() : kv_pair(), ptr(), brother(0) {
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
                if (cursor.kv_pair[mid] > kv) {
                    r = mid;
                } else {
                    l = mid + 1;
                }
            }

            if (l > 0 && cursor.kv_pair[l - 1] == kv)
                return false;

            if (cursor.size < MAX) {
                for (int i = cursor.size - 1; i >= l; --i) {
                    cursor.kv_pair[i + 1] = cursor.kv_pair[i];
                }
                cursor.size++;
                cursor.kv_pair[l] = kv;
                MR.update(cursor, pos);
                return false;
            }

            for (int i = cursor.size - 1; i >= l; --i) {
                cursor.kv_pair[i + 1] = cursor.kv_pair[i];
            }
            cursor.size++;
            cursor.kv_pair[l] = kv;
            int newpos = MR.get_index();

            static Node newbro;
            newbro.Is_leaf = true;
            newbro.size = MIN + 1;
            newbro.brother = cursor.brother;
            cursor.brother = newpos;
            for (int i = 0; i <= MIN; ++i) {
                newbro.kv_pair[i] = cursor.kv_pair[i + MIN];
            }

            cursor.size = MIN;
            if (root == pos) {

                static Node newroot;
                newroot.Is_leaf = false;
                newroot.size = 1;
                newroot.kv_pair[0] = cursor.kv_pair[MIN];
                newroot.ptr[0] = pos;
                newroot.ptr[1] = newpos;

                MR.update(cursor, pos);
                MR.update(newbro, newpos);
                int rootpos = MR.get_index();
                MR.write(newroot);
                root = rootpos;
                return false;
            }
            MR.update(cursor, pos);
            MR.update(newbro, newpos);
            pass = newbro.kv_pair[0];
            return true;
        }

        int l = 0, r = cursor.size;
        while (l < r) {
            int mid = (l + r) >> 1;
            if (cursor.kv_pair[mid] >= kv) {
                r = mid;
            } else {
                l = mid + 1;
            }
        }

        Node child;
        MR.read(child, cursor.ptr[l]);

        bool state = insertInternal(child, cursor.ptr[l], kv);
        if (!state)
            return false;
        if (cursor.size < MAX) {
            for (int i = cursor.size - 1; i >= l; --i) {
                cursor.kv_pair[i + 1] = cursor.kv_pair[i];
                cursor.ptr[i + 2] = cursor.ptr[i + 1];
            }
            cursor.size++;
            cursor.kv_pair[l] = pass;
            cursor.ptr[l + 1] = MR.get_index()-1;
            MR.update(cursor, pos);
            return false;
        }
        for (int i = cursor.size - 1; i >= l; --i) {
            cursor.kv_pair[i + 1] = cursor.kv_pair[i];
            cursor.ptr[i + 2] = cursor.ptr[i + 1];
        }
        ++cursor.size;
        cursor.kv_pair[l] = pass;
        cursor.ptr[l + 1] = MR.get_index()-1;

        int newpos = MR.get_index();
        pass = cursor.kv_pair[MIN];
        static Node newbro;
        newbro.Is_leaf = false;
        newbro.size = MIN;
        for (int i = 0; i < MIN; ++i) {
            newbro.kv_pair[i] = cursor.kv_pair[i + MIN + 1];
            newbro.ptr[i] = cursor.ptr[i + MIN + 1];
        }
        newbro.ptr[MIN] = cursor.ptr[cursor.size];

        cursor.size = MIN;
        if (root == pos) {
            static Node newroot;
            newroot.Is_leaf = false;
            newroot.size = 1;
            newroot.kv_pair[0] = pass;
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
                if (cursor.kv_pair[mid] > kv) {
                    r = mid;
                } else {
                    l = mid + 1;
                }
            }
            --l;
            if (l < 0 || l >= cursor.size || cursor.kv_pair[l] != kv) {
                return false;
            }
            for (int i = l + 1; i < cursor.size; ++i) {
                cursor.kv_pair[i - 1] = cursor.kv_pair[i];
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
            if (cursor.kv_pair[mid] >= kv) {
                r = mid;
            } else {
                l = mid + 1;
            }
        }
        if (l < cursor.size && kv == cursor.kv_pair[l]) {
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
                        newbro[0].kv_pair[i + newbro[0].size] = newbro[1].kv_pair[i];
                    }
                    newbro[0].size += newbro[1].size;
                    newbro[0].brother = newbro[1].brother;
                    root = cursor.ptr[0];
                    MR.update(newbro[0], cursor.ptr[0]);
                    return false;
                }
                for (int i = 0; i < newbro[1].size; ++i) {
                    newbro[0].kv_pair[i + newbro[0].size + 1] = newbro[1].kv_pair[i];
                    newbro[0].ptr[i + newbro[0].size + 1] = newbro[1].ptr[i];
                }
                newbro[0].ptr[newbro[0].size + newbro[1].size + 1] = newbro[1].ptr[newbro[1].size];
                newbro[0].kv_pair[newbro[0].size] = cursor.kv_pair[0];
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
                        child.kv_pair[i + 1] = child.kv_pair[i];
                    }
                    child.kv_pair[0] = newbro.kv_pair[newbro.size - 1];
                    ++child.size;
                    --newbro.size;
                    cursor.kv_pair[l - 1] = child.kv_pair[0];
                    MR.update(cursor, pos);
                    MR.update(newbro, cursor.ptr[l - 1]);
                    MR.update(child, cursor.ptr[l]);
                    return false;
                }
                MR.read(newbro, cursor.ptr[l - 1]);
                for (int i = child.size; i >= 1; --i) {
                    child.kv_pair[i] = child.kv_pair[i - 1];
                    child.ptr[i + 1] = child.ptr[i];
                }
                child.ptr[1] = child.ptr[0];
                ++child.size;
                child.kv_pair[0] = cursor.kv_pair[l - 1];
                child.ptr[0] = newbro.ptr[newbro.size];
                cursor.kv_pair[l - 1] = newbro.kv_pair[newbro.size - 1];
                --newbro.size;
                MR.update(cursor, pos);
                MR.update(newbro, cursor.ptr[l - 1]);
                MR.update(child, cursor.ptr[l]);
                return false;
            }
            if (child.Is_leaf) {
                MR.read(newbro, cursor.ptr[l - 1]);
                for (int i = 0; i < child.size; ++i) {
                    newbro.kv_pair[i + newbro.size] = child.kv_pair[i];
                }
                newbro.size += child.size;
                newbro.brother = child.brother;
                for (int i = l; i < cursor.size; ++i) {
                    cursor.kv_pair[i - 1] = cursor.kv_pair[i];
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
                newbro.kv_pair[i + newbro.size + 1] = child.kv_pair[i];
                newbro.ptr[i + newbro.size + 1] = child.ptr[i];
            }
            newbro.ptr[newbro.size + child.size + 1] = child.ptr[child.size];
            newbro.kv_pair[newbro.size] = cursor.kv_pair[l - 1];
            newbro.size += child.size + 1;
            for (int i = l - 1; i < cursor.size - 1; ++i) {
                cursor.kv_pair[i] = cursor.kv_pair[i + 1];
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
                    child.kv_pair[child.size] = newbro.kv_pair[0];
                    ++child.size;
                    for (int i = 0; i < newbro.size - 1; ++i) {
                        newbro.kv_pair[i] = newbro.kv_pair[i + 1];
                    }
                    --newbro.size;
                    cursor.kv_pair[l] = newbro.kv_pair[0];
                    MR.update(cursor, pos);
                    MR.update(child, cursor.ptr[l]);
                    MR.update(newbro, cursor.ptr[l + 1]);
                    return false;
                }
                MR.read(newbro, cursor.ptr[l + 1]);
                child.kv_pair[child.size] = cursor.kv_pair[l];
                child.ptr[child.size + 1] = newbro.ptr[0];
                ++child.size;
                cursor.kv_pair[l] = newbro.kv_pair[0];
                for (int i = 0; i < newbro.size - 1; ++i) {
                    newbro.kv_pair[i] = newbro.kv_pair[i + 1];
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
                    child.kv_pair[i + child.size] = newbro.kv_pair[i];
                }
                child.size += newbro.size;
                child.brother = newbro.brother;
                for (int i = l; i < cursor.size - 1; ++i) {
                    cursor.kv_pair[i] = cursor.kv_pair[i + 1];
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
                child.kv_pair[i + child.size + 1] = newbro.kv_pair[i];
                child.ptr[i + child.size + 1] = newbro.ptr[i];
            }
            child.ptr[child.size + newbro.size + 1] = newbro.ptr[newbro.size];
            child.kv_pair[child.size] = cursor.kv_pair[l];
            child.size += newbro.size + 1;
            for (int i = l; i < cursor.size - 1; ++i) {
                cursor.kv_pair[i] = cursor.kv_pair[i + 1];
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
        if (root == -1) {//空树
            Node x;
            x.kv_pair[0] = kv;
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
                if (index <= cursor.kv_pair[i].index && (i - 1 == -1 || index >= cursor.kv_pair[i - 1].index)) {
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
            if (index < cursor.kv_pair[i].index) {
                break;
            }
            if (index == cursor.kv_pair[i].index) {
                ans.push_back(cursor.kv_pair[i].value);
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
