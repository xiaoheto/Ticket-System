#include <string>
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

template<class K, class T>
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

template<typename K, typename T, int MAX = 102, int MIN = MAX / 2>
class BPTree {
    struct Node {
        bool is_leaf;
        int size;
        KVPair<K, T> kv_pair[MAX + 2];
        int son[MAX + 2];
        int sib;

        Node() : is_leaf(false),size(0), kv_pair(),sib(0) {
            for (int i = 0; i < MAX + 1; ++i) {
                son[i] = 0;
            }
        }
    };

    MemoryRiver<Node> base_file;
    KVPair<K, T> pass;
    int root;

    // find the first pos that is larger than kv
    static int findPos(int l, int r, Node cur_node, KVPair<K, T> kv) {
        while (l < r) {
            int mid = (l + r) / 2;
            if (cur_node.kv_pair[mid] > kv) {
                r = mid;
            } else {
                l = mid + 1;
            }
        }
        return l;
    }

    bool Insert(Node cur_node, int pos, KVPair<K, T> kv) {
        if (cur_node.is_leaf) {
            int l = findPos(0, cur_node.size, cur_node, kv);

            if (l > 0 && cur_node.kv_pair[l - 1] == kv) {
                return false;
            } // already exists

            for (int i = cur_node.size - 1; i >= l; --i) {
                cur_node.kv_pair[i + 1] = cur_node.kv_pair[i];
            }

            if (cur_node.size < MAX) {
                ++cur_node.size;
                cur_node.kv_pair[l] = kv;
                base_file.update(cur_node, pos);
                return false;
            }

            ++cur_node.size;
            cur_node.kv_pair[l] = kv;
            int new_pos = base_file.get_index();

            static Node new_sib;
            new_sib.is_leaf = true;
            new_sib.size = MIN + 1;
            new_sib.sib = cur_node.sib;
            cur_node.sib = new_pos;
            for (int i = 0; i <= MIN; ++i) {
                new_sib.kv_pair[i] = cur_node.kv_pair[i + MIN];
            }

            cur_node.size = MIN;
            if (root == pos) {
                static Node new_root;
                new_root.is_leaf = false;
                new_root.size = 1;
                new_root.kv_pair[0] = cur_node.kv_pair[MIN];
                new_root.son[0] = pos;
                new_root.son[1] = new_pos;

                base_file.update(cur_node, pos);
                base_file.update(new_sib, new_pos);
                int root_pos = base_file.get_index();
                base_file.write(new_root);
                root = root_pos;
                return false;
            }
            base_file.update(cur_node, pos);
            base_file.update(new_sib, new_pos);
            pass = new_sib.kv_pair[0];
            return true;
        }

        int l = findPos(0, cur_node.size, cur_node, kv);

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
            ++cur_node.size;
            cur_node.kv_pair[l] = pass;
            cur_node.son[l + 1] = base_file.get_index() - 1;
            base_file.update(cur_node, pos);
            return false;
        }
        for (int i = cur_node.size - 1; i >= l; --i) {
            cur_node.kv_pair[i + 1] = cur_node.kv_pair[i];
            cur_node.son[i + 2] = cur_node.son[i + 1];
        }
        ++cur_node.size;
        cur_node.kv_pair[l] = pass;
        cur_node.son[l + 1] = base_file.get_index() - 1;

        int new_pos = base_file.get_index();
        pass = cur_node.kv_pair[MIN];
        static Node new_sib;
        new_sib.is_leaf = false;
        new_sib.size = MIN;
        for (int i = 0; i < MIN; ++i) {
            new_sib.kv_pair[i] = cur_node.kv_pair[i + MIN + 1];
            new_sib.son[i] = cur_node.son[i + MIN + 1];
        }
        new_sib.son[MIN] = cur_node.son[cur_node.size];

        cur_node.size = MIN;
        if (root == pos) {
            static Node new_root;
            new_root.is_leaf = false;
            new_root.size = 1;
            new_root.kv_pair[0] = pass;
            new_root.son[0] = pos;
            new_root.son[1] = new_pos;
            base_file.update(cur_node, pos);
            base_file.update(new_sib, new_pos);
            root = base_file.write(new_root);
            return false;
        }
        base_file.update(cur_node, pos);
        base_file.update(new_sib, new_pos);
        return true;
    }

    bool Delete(Node &cur_node, int pos, const KVPair<K, T> &kv) {
        if (cur_node.is_leaf) {
            int l = 0, r = cur_node.size;
            while (l < r) {
                int mid = (l + r) >> 1;
                if (cur_node.kv_pair[mid] > kv) {
                    r = mid;
                } else {
                    l = mid + 1;
                }
            }
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

        int l = findPos(0, cur_node.size, cur_node, kv);

        if (l < cur_node.size && kv == cur_node.kv_pair[l]) {
            ++l;
        }
        Node child;
        base_file.read(child, cur_node.son[l]);
        bool state = Delete(child, cur_node.son[l], kv);
        if (!state)
            return false;
        if (pos == root && cur_node.size == 1) {
            static Node new_sib[2];
            base_file.read(new_sib[0], cur_node.son[0]);
            base_file.read(new_sib[1], cur_node.son[1]);
            if (new_sib[0].size + new_sib[1].size < MAX) {
                base_file.read(new_sib[0], cur_node.son[0]);
                base_file.read(new_sib[1], cur_node.son[1]);
                if (new_sib[0].is_leaf) {
                    for (int i = 0; i < new_sib[1].size; ++i) {
                        new_sib[0].kv_pair[i + new_sib[0].size] = new_sib[1].kv_pair[i];
                    }
                    new_sib[0].size += new_sib[1].size;
                    new_sib[0].sib = new_sib[1].sib;
                    root = cur_node.son[0];
                    base_file.update(new_sib[0], cur_node.son[0]);
                    return false;
                }
                for (int i = 0; i < new_sib[1].size; ++i) {
                    new_sib[0].kv_pair[i + new_sib[0].size + 1] = new_sib[1].kv_pair[i];
                    new_sib[0].son[i + new_sib[0].size + 1] = new_sib[1].son[i];
                }
                new_sib[0].son[new_sib[0].size + new_sib[1].size + 1] = new_sib[1].son[new_sib[1].size];
                new_sib[0].kv_pair[new_sib[0].size] = cur_node.kv_pair[0];
                new_sib[0].size += new_sib[1].size + 1;
                root = cur_node.son[0];
                base_file.update(new_sib[0], cur_node.son[0]);
                return false;
            }
        }
        if (l > 0) {
            static Node new_sib;
            base_file.read(new_sib, cur_node.son[l - 1]);
            if (new_sib.size > MIN) {
                if (child.is_leaf) {
                    base_file.read(new_sib, cur_node.son[l - 1]);
                    for (int i = child.size - 1; i >= 0; --i) {
                        child.kv_pair[i + 1] = child.kv_pair[i];
                    }
                    child.kv_pair[0] = new_sib.kv_pair[new_sib.size - 1];
                    ++child.size;
                    --new_sib.size;
                    cur_node.kv_pair[l - 1] = child.kv_pair[0];
                    base_file.update(cur_node, pos);
                    base_file.update(new_sib, cur_node.son[l - 1]);
                    base_file.update(child, cur_node.son[l]);
                    return false;
                }
                base_file.read(new_sib, cur_node.son[l - 1]);
                for (int i = child.size; i >= 1; --i) {
                    child.kv_pair[i] = child.kv_pair[i - 1];
                    child.son[i + 1] = child.son[i];
                }
                child.son[1] = child.son[0];
                ++child.size;
                child.kv_pair[0] = cur_node.kv_pair[l - 1];
                child.son[0] = new_sib.son[new_sib.size];
                cur_node.kv_pair[l - 1] = new_sib.kv_pair[new_sib.size - 1];
                --new_sib.size;
                base_file.update(cur_node, pos);
                base_file.update(new_sib, cur_node.son[l - 1]);
                base_file.update(child, cur_node.son[l]);
                return false;
            }
            if (child.is_leaf) {
                base_file.read(new_sib, cur_node.son[l - 1]);
                for (int i = 0; i < child.size; ++i) {
                    new_sib.kv_pair[i + new_sib.size] = child.kv_pair[i];
                }
                new_sib.size += child.size;
                new_sib.sib = child.sib;
                for (int i = l; i < cur_node.size; ++i) {
                    cur_node.kv_pair[i - 1] = cur_node.kv_pair[i];
                    cur_node.son[i] = cur_node.son[i + 1];
                }
                --cur_node.size;
                new_sib.sib = child.sib;
                base_file.update(cur_node, pos);
                base_file.update(new_sib, cur_node.son[l - 1]);
                if (cur_node.size < MIN)
                    return true;
                return false;
            }
            base_file.read(new_sib, cur_node.son[l - 1]);
            for (int i = 0; i < child.size; ++i) {
                new_sib.kv_pair[i + new_sib.size + 1] = child.kv_pair[i];
                new_sib.son[i + new_sib.size + 1] = child.son[i];
            }
            new_sib.son[new_sib.size + child.size + 1] = child.son[child.size];
            new_sib.kv_pair[new_sib.size] = cur_node.kv_pair[l - 1];
            new_sib.size += child.size + 1;
            for (int i = l - 1; i < cur_node.size - 1; ++i) {
                cur_node.kv_pair[i] = cur_node.kv_pair[i + 1];
                cur_node.son[i + 1] = cur_node.son[i + 2];
            }
            --cur_node.size;
            base_file.update(cur_node, pos);
            base_file.update(new_sib, cur_node.son[l - 1]);
            if (cur_node.size < MIN)
                return true;
            return false;
        } else if (l < cur_node.size) {
            static Node new_sib;
            base_file.read(new_sib, cur_node.son[l + 1]);
            if (new_sib.size > MIN) {
                if (child.is_leaf) {
                    base_file.read(new_sib, cur_node.son[l + 1]);
                    child.kv_pair[child.size] = new_sib.kv_pair[0];
                    ++child.size;
                    for (int i = 0; i < new_sib.size - 1; ++i) {
                        new_sib.kv_pair[i] = new_sib.kv_pair[i + 1];
                    }
                    --new_sib.size;
                    cur_node.kv_pair[l] = new_sib.kv_pair[0];
                    base_file.update(cur_node, pos);
                    base_file.update(child, cur_node.son[l]);
                    base_file.update(new_sib, cur_node.son[l + 1]);
                    return false;
                }
                base_file.read(new_sib, cur_node.son[l + 1]);
                child.kv_pair[child.size] = cur_node.kv_pair[l];
                child.son[child.size + 1] = new_sib.son[0];
                ++child.size;
                cur_node.kv_pair[l] = new_sib.kv_pair[0];
                for (int i = 0; i < new_sib.size - 1; ++i) {
                    new_sib.kv_pair[i] = new_sib.kv_pair[i + 1];
                    new_sib.son[i] = new_sib.son[i + 1];
                }
                new_sib.son[new_sib.size - 1] = new_sib.son[new_sib.size];
                --new_sib.size;
                base_file.update(cur_node, pos);
                base_file.update(child, cur_node.son[l]);
                base_file.update(new_sib, cur_node.son[l + 1]);
                return false;
            }
            if (child.is_leaf) {
                base_file.read(new_sib, cur_node.son[l + 1]);
                for (int i = 0; i < new_sib.size; ++i) {
                    child.kv_pair[i + child.size] = new_sib.kv_pair[i];
                }
                child.size += new_sib.size;
                child.sib = new_sib.sib;
                for (int i = l; i < cur_node.size - 1; ++i) {
                    cur_node.kv_pair[i] = cur_node.kv_pair[i + 1];
                    cur_node.son[i + 1] = cur_node.son[i + 2];
                }
                --cur_node.size;
                child.sib = new_sib.sib;
                base_file.update(cur_node, pos);
                base_file.update(child, cur_node.son[l]);
                if (cur_node.size < MIN)
                    return true;
                return false;
            }
            base_file.read(new_sib, cur_node.son[l + 1]);
            for (int i = 0; i < new_sib.size; ++i) {
                child.kv_pair[i + child.size + 1] = new_sib.kv_pair[i];
                child.son[i + child.size + 1] = new_sib.son[i];
            }
            child.son[child.size + new_sib.size + 1] = new_sib.son[new_sib.size];
            child.kv_pair[child.size] = cur_node.kv_pair[l];
            child.size += new_sib.size + 1;
            for (int i = l; i < cur_node.size - 1; ++i) {
                cur_node.kv_pair[i] = cur_node.kv_pair[i + 1];
                cur_node.son[i + 1] = cur_node.son[i + 2];
            }
            --cur_node.size;
            base_file.update(cur_node, pos);
            base_file.update(child, cur_node.son[l]);
            if (cur_node.size < MIN)
                return true;
            return false;
        }
        throw;
    }

public:
    BPTree() {
        root = -1;
        base_file.initialise("BPTree.dat");
        base_file.get_info(root, 1);
    }

    ~BPTree() {
        base_file.write_info(root, 1);
        base_file.end();
    };

    bool empty() const{
        return root == -1;
    }

    void insert(K index, T value) {
        KVPair<K, T> kv(index, value);
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

    void erase(K index, T value) {
        if (root == -1) {
            return;
        }
        KVPair<K, T> kv(index, value);
        Node cur_node;
        base_file.read(cur_node, root);
        Delete(cur_node, root, kv);
    }

    vector<T> query(K index) {
        vector<T> ans;
        ans.clear();
        if (root == -1) {
            return ans;
        }
        Node cur_node;
        base_file.read(cur_node, root);
        while (!cur_node.is_leaf) {
            int i = 0;
            for (; i < cur_node.size; i++) {
                if ((index <= cur_node.kv_pair[i].index) && (i - 1 == -1 || index >= cur_node.kv_pair[i - 1].index)) {
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

    void end() {
        base_file.end();
    }
};
