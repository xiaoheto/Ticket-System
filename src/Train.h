//
// Created by 43741 on 2025/5/15.
//

#ifndef TRAIN_H
#define TRAIN_H
#include <iostream>
#include "MyChar.h"
#include "Time.h"

class Station {

    MyChar<24> station_name;
    int price_to_next; // 到下一站的票价
    int arrive_time; // 到达这一站的时间
    int leave_time; // 离开这一站的时间

public:

    Station() = default;

    Station(const MyChar<24> &name) :station_name(name) {}

    Station(const MyChar<24> name,int price,int arr,int lea):
    station_name(name),price_to_next(price),arrive_time(arr),leave_time(lea) {}
};

class Train {

    MyChar<24> trainID;
    int stationNum;
    Station stations[100];
    int seatNum;
    int startTime;
    Day saleDate;
    MyChar<2> type;

public:

    int add_train(MyChar<24>trainID,);
};
#endif //TRAIN_H
