#pragma once
#ifndef MyDataFrame_hpp
#define MyDataFrame_hpp
/*
#ifndef string
#include <string>
#endif  <string> */
#include <string>
#include <stdio.h>
#include <vector>
#include <map>
#include <time.h>
#include <algorithm>

#include "DataHandler.hpp"

using namespace std;

class MyDataFrame {
public:
    map<string, map<long, double>> price_tickers;  // {"close":{(time, price),(time, price),...}, "vwap":{(time, price),(time, price),...}}
    //map<string, map<long, map<double, double>>> order_books;  // {"ask":{"t0":{(price, volume),(price, volume),..}}, "bid":{"t0":{(price, volume),(price, volume),..}}}
    vector<long> epoch_list;
    long last_period;
    map<string, map<long, double>> last_price_tickers;
    //map<string, map<long, map<double, double>>> last_order_books;

    vector<long> last_epoch_list;
    string symbol;
    MyDataFrame(string symbl, long last_period);
    MyDataFrame();
    void load_data_from_cloud(string* start_date, string* end_date);
    void load_last_data(long* start_epoch, long* end_epoch);
};

//long get_epoch_time(string date);
//string get_std_time(long epochtime);

#endif /* MarketDataFrame_hpp */
