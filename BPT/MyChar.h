//
// Created by 43741 on 2025/5/11.
//

#ifndef MYSTRING_H
#define MYSTRING_H
#include <cstring>
#include <iostream>
#include <iomanip>
using std::string;
using std::strcpy;

template <int length>
class MyChar {

    char myChar[length];

public:
    MyChar() = default;

    explicit MyChar(const string &str) {
        strcpy(myChar,str.c_str());
    };

    MyChar(const MyChar &other) {
        strcpy(myChar,other.myChar);
    };

    explicit MyChar(char *ch) {
        strcpy(myChar,ch);
    }

    const char* c_str() const {
        return myChar;
    }

    MyChar &operator=(const MyChar &other) {
        if (this != &other) {
            strcpy(myChar,other.myChar);
        }
        return *this;
    }

    bool operator==(const MyChar &other) const{
        return strcmp(myChar,other.myChar) == 0;
    }

    bool operator!=(const MyChar &other) const{
        return !(*this == other);
    }

    bool operator<(const MyChar &other) const{
        return strcmp(myChar,other.myChar) < 0;
    }

    bool operator>(const MyChar &other) const{
        return strcmp(myChar,other.myChar) > 0;
    }

    bool operator<=(const MyChar &other) const{
        return *this == other || *this < other;
    }

    bool operator>=(const MyChar &other) const{
        return *this == other || *this > other;
    }

    friend std::istream &operator>>(std::istream &in,MyChar &obj) {
        in >> obj.myChar;
        return in;
    }
};
#endif //MYSTRING_H
