//
// Created by 43741 on 2025/5/15.
//

#ifndef TIME_H
#define TIME_H
#include <iostream>
using std::ostream;
class Clock {

    int hour;
    int minute;

public:
    Clock() = default;

    Clock(int h,int m);

    Clock(const Clock &other);

    Clock &operator=(const Clock &other);

    friend ostream &operator<<(ostream &out,const Clock &obj);

    bool operator<(const Clock &other) const;

    bool operator==(const Clock &other) const;

    int operator-(const Clock &other);
};

class Day {

    int month;
    int date;

public:
    Day() = default;

    Day(int m,int d);

    Day &operator=(const Day &other);

    bool operator<(const Day &other) const;

    bool operator==(const Day &other) const;
};
#endif //TIME_H
