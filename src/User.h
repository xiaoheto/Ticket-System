//
// Created by 43741 on 2025/5/10.
//

#ifndef USER_H
#define USER_H
#include <iostream>
#include "MyChar.h"
using std::ostream;
class User {
    MyChar<24> username;
    MyChar<32> password;
    MyChar<24> name;
    MyChar<32> mailAddr;
    int privilege = -1;

public:
    User() = default;

    User(const MyChar<24> &user_name,const MyChar<32> &pass_word,const MyChar<24> &na_me,const MyChar<32> &mail_addr,int pri) {
        privilege = pri;
        username = user_name;
        password = pass_word;
        name = na_me;
        mailAddr = mail_addr;
    }

    //<username>，<name>，<mailAddr> 和 <privilege>
    friend ostream &operator<<(ostream &out,const User &obj) {
        out << obj.username << ' ' << obj.name << ' ' << obj.mailAddr << ' ' << obj.privilege << '\n';
        return out;
    }
};
#endif //USER_H
