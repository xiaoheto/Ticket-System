//
// Created by 43741 on 2025/5/15.
//

#include "Time.h"

Clock::Clock(int h, int m):hour(h),minute(m) {}

Clock::Clock(const Clock &other):hour(other.hour),minute(other.minute) {}

Clock &Clock::operator=(const Clock &other) {
    if (this != &other) {
        hour = other.hour;
        minute = other.minute;
    }
    return *this;
}

ostream &operator<<(ostream &out, const Clock &obj) {
    out << obj.hour << ':' << obj.minute;
    return out;
}

bool Clock::operator<(const Clock &other) const {
    if (hour == other.hour) {
        return minute < other.minute;
    }
    return hour < other.hour;
}

bool Clock::operator==(const Clock &other) const {
    return hour == other.hour && minute == other.minute;
}

int Clock::operator-(const Clock &other) {
    return (hour - other.hour) * 60 + minute - other.minute;
}

Day::Day(int m, int d):month(m),date(d) {}

Day &Day::operator=(const Day &other) {
    if (this != &other) {
        month = other.month;
        date = other.date;
    }
    return *this;
}

bool Day::operator<(const Day &other) const {
    if (month == other.month) {
        return date < other.date;
    }
    return month < other.month;
}

bool Day::operator==(const Day &other) const {
    return month == other.month && date == other.date;
}