//
// Created by 43741 on 2025/5/11.
//

#include "CommandParser.h"

#include "../src/MyError.h"

Command::Command(const Command &other):
input(other.input),position(other.position),split_char(other.split_char) {}

Command::Command(char split):split_char(split),position(0) {}

Command::Command(const string &input, char split):
input(input),split_char(split),position(0) {}

int Command::string_to_int(const string &s) {
    int ans = 0;
    for (int i = 0;i < s.size();++i) {
        if (s[i] == '\r' || s[i] == '\n') {
            break;
        }
        if (s[i] < '0' || s[i] > '9') {
            throw Error("Invalid\n");
        }
        ans *= 10;
        ans += s[i] - '0';
    }
    return ans;
}

void Command::countArguments() {
    count = 0;
    int i = 0;
    while(i < input.length() && input[i] != '\r' && input[i] != '\n') {
        while(i < input.length() && input[i] == split_char) {
            ++i;
        }

        if (i < input.length() && input[i] != '\r' && input[i] != '\n') {
            ++count;
            while (i < input.length() && input[i] != split_char && input[i] != '\r' && input[i] != '\n') {
                i++;
            }
        }
    }
}

string Command::getNext() {
    if (position >= input.length() || input[position] == '\r' || input[position] == '\n') {
        return "";
    }
    string result;
    while(position < input.length() && input[position] != split_char && input[position] != '\r' && input[position] != '\n') {
        result += input[position];
        ++position;
    }

    while(position < input.length() && input[position] == split_char) {
        ++position;
    } // 移动到下一个单词开始

    return result;
}

void Command::reset() {
    input.clear();
    position = 0;
    count = 0;
    split_char = ' ';
}

void Command::setSplit(char new_split) {
    split_char = new_split;
    position = 0;
    countArguments();
}

istream &operator>>(istream &in,Command &obj) {
    std::getline(in,obj.input);
    obj.position = 0;
    while(obj.position < obj.input.length() && obj.input[obj.position] == obj.split_char) {
        ++obj.position;
    }
    obj.countArguments();
    return in;
}
