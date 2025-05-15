#include <iostream>
#include <string>
#include <sstream>

#include "CommandParser.h"
#include "STLite/vector.hpp"
#include "BPT/BPlusTree.hpp"
#include "MyChar.h"
using std::cin;
using std::cout;
using sjtu::vector;
using std::string;
using std::cin;
using std::cout;
using std::ios;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    cout.tie(nullptr);

    bool flag = false; // 是否是第一个用户

    string input;
    while(getline(cin,input)) {
        try {
            Command command(input);
            if (command.count == 0) {
                continue;
            }
            string timestamp = command.getNext();
            string cmd = command.getNext();
            // 当前用户 -c（<cur_username>），
            // 创建一个账户名为 -u（<username>），
            // 密码为 -p（<password>），
            // 用户真实姓名为 -n（<name>），
            // 用户邮箱为 -m（<mailAddr>），
            // 权限为 -g（<privilege>）的用户
            if (cmd == "add_user") {
                if (!flag) {
                } // 第一个用户登录
                else{
                    string c = command.getNext(); MyChar<32> cur_username = command.getNext();
                    string u = command.getNext(); MyChar<32> username = command.getNext();
                    string p = command.getNext(); MyChar<32> password = command.getNext();
                    string n = command.getNext(); MyChar<24> name = command.getNext();
                    string m = command.getNext(); MyChar<32> mailAddr = command.getNext();
                    string g = command.getNext(); string pri = command.getNext(); int privilege = command.string_to_int(pri);
                }
            }
        } catch(...) {
        }
    }
}