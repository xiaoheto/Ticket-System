//
// Created by 43741 on 2025/5/11.
//

#ifndef COMMANDPARSER_H
#define COMMANDPARSER_H
#include <random>
#include <string>
#include <sstream>
#include "../STLite/vector.hpp"
using std::string;
using std::istream;

class Command {

    string input = "";
    int position;
    char split_char;

public:
    int count = 0;

    Command() = default;

    Command(const Command &other);

    Command(const string &input,char split = ' ');

    Command(char split);

    ~Command() = default;

    int string_to_int(const string &s);

    void countArguments();

    string getNext();

    void reset();

    void setSplit(char new_split);

    friend istream &operator>>(istream &in,Command &obj);
};
#endif //COMMANDPARSER_H
