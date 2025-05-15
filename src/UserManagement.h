//
// Created by 43741 on 2025/5/14.
//

#ifndef USERMANAGEMENT_H
#define USERMANAGEMENT_H
#include "User.h"
#include "BPlusTree.hpp"
#include "map.hpp"

using sjtu::map;

class UserManagement {

    BPlusTree<User,120,60,24> UserInfo;
    MemoryRiver<User> UserFile;
    map<MyChar<24>,int> LogInStack; // username->privilege
    int count = 0;

public:

    UserManagement() {
        UserFile.initialise("User_file");
        UserFile.get_info(count,1);
    }

    int add_first_user(const MyChar<24> &username,const MyChar<32> &password,const MyChar<24> &name,const MyChar<32> &mailAddr);

    int add_user(const MyChar<24> &cur_username,const MyChar<32> &username,const MyChar<32> & password,const MyChar<24> &name,const MyChar<32> &mailAddr,int privilege);

    int login(const MyChar<24> &username,const MyChar<32> &password);

    int logout(const MyChar<24> &username);

    User query_profile(const MyChar<24> &cur_username,const MyChar<24> &username);

    User modify_profile(const MyChar<24> &cur_username,const MyChar<24> &username,const MyChar<43> &password,const MyChar<24> &name,const MyChar<32> &mailAddr,int privilege);
};
#endif //USERMANAGEMENT_H
