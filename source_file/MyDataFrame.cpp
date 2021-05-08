//
//  MarketDataFrame.cpp
//  Backtest Environment
//

#ifndef fstream
#include <fstream>
#endif /* <fstream> */
#ifndef iostream
#include <iostream>
#endif /* <iostream> */
#ifndef sstream
#include <sstream>
#endif /* <sstream> */
#include <stdio.h>
#include "MyDataFrame.hpp"
#include <time.h>

map<string, map<long, double>> data;
vector<long> indices;
map<string, map<long, double>> price_tickers;  // {"close":{(time, price),(time, price),...}, "vwap":{(time, price),(time, price),...}}
//map<string, map<long, map<double, double>>> order_books;  // {"ask":{"t0":{(price, volume),(price, volume),..}}, "bid":{"t0":{(price, volume),(price, volume),..}}}

// Placeholder default constructor
MyDataFrame::MyDataFrame() {}

// Constructor
MyDataFrame::MyDataFrame(string symbl, long _last_period) : symbol(symbl),last_period(_last_period) {}

// Load data from cloud
void MyDataFrame::load_data_from_cloud(string* start_date, string* end_date) {
    cout << "Load data of " << symbol << " from " << *start_date << " to " << *end_date << "...\n";
    std::string host = "43.128.8.191";


    auto sqlmgr = new Mysqlmgr();
    
    if (!sqlmgr->connect(host.c_str(), "root", "123456", "btc", 0, 0, 0))
    {
        printf("can not connect to mysql: %s\n", sqlmgr->geterror());
       // return;
    }
    printf("connected sucess!\n");
    long epoch_start = get_epoch_time(*start_date);
    cout << "MyDataFrame_cpp load_data_from_cloud" << endl;
    string actual_start_date = get_std_time(epoch_start - last_period);
    
    price_tickers["close"] = load_price_tickers_from_cloud(&actual_start_date, end_date, sqlmgr);
    //order_books["ask"] = load_ask_orders_from_cloud(&actual_start_date, end_date, sqlmgr);
    //order_books["bid"] = load_bid_orders_from_cloud(&actual_start_date, end_date, sqlmgr);
    for (auto iter = price_tickers["close"].begin(); iter != price_tickers["close"].end(); iter++)
    {
        if (iter->first < epoch_start) last_epoch_list.push_back(iter->first);
        else epoch_list.push_back(iter->first);
    }
    delete sqlmgr;
}

void MyDataFrame::load_last_data(long* start_epoch,long* end_epoch)
{
    if (*end_epoch < epoch_list[0])
    {
        std::string host = "43.128.8.191";
        auto sqlmgr = new Mysqlmgr();
        if (!sqlmgr->connect(host.c_str(), "root", "123456", "btc", 0, 0, 0))
        {
            printf("can not connect to mysql: %s\n", sqlmgr->geterror());
            return;
        }
        printf("connected sucess!\n");
        
        cout << "MyDataFrame_cpp load_last_data..."<<endl;
        auto start_date = get_std_time(*start_epoch);
        auto end_date = get_std_time(*end_epoch);
        last_price_tickers["close"] = load_price_tickers_from_cloud(&start_date, &end_date, sqlmgr);
        //last_order_books["ask"] = load_ask_orders_from_cloud(&start_date, &end_date, sqlmgr);
        //last_order_books["bid"] = load_bid_orders_from_cloud(&start_date, &end_date, sqlmgr);
        for (auto iter = price_tickers["close"].begin(); iter != price_tickers["close"].end(); iter++)
        {
            last_epoch_list.push_back(iter->first);
        }
        delete sqlmgr;
    }
    else if (*end_epoch >= epoch_list[0])
    {
        // delete old data
        for (auto iter = last_price_tickers["close"].begin(); iter != last_price_tickers["close"].end(); iter++)
        {
            if (iter->first < *start_epoch)
                last_price_tickers["close"].erase(iter);
            else
                break;
        }
        /*
        for (auto iter = last_order_books["ask"].begin(); iter != last_order_books["ask"].end(); iter++)
        {
            if (iter->first < *start_epoch)
                order_books["ask"].erase(iter);
            else
                break;
        }
        for (auto iter = last_order_books["bid"].begin(); iter != last_order_books["bid"].end(); iter++)
        {
            if (iter->first < *start_epoch)
                order_books["bid"].erase(iter);
            else
                break;
        }
        */
        // extend the last period data
        long end_epoch_tmp = last_epoch_list.back();
        for (auto iter = price_tickers["close"].begin(); iter != price_tickers["close"].end(); iter++)
        {
            if (iter->first <= *end_epoch) {
                if (iter->first > end_epoch_tmp)
                    last_price_tickers["close"].insert(make_pair(iter->first, iter->second));
            }
            else
                break;
        }
        /*
        for (auto iter = order_books["ask"].begin(); iter != order_books["ask"].end(); iter++)
        {
            if (iter->first <= *end_epoch) {
                if (iter->first > end_epoch_tmp)
                    last_order_books["ask"].insert(make_pair(iter->first, iter->second));
            }
            else
                break;
        }
        for (auto iter = order_books["bid"].begin(); iter != order_books["bid"].end(); iter++)
        {
            if (iter->first <= *end_epoch) {
                if (iter->first > end_epoch_tmp)
                    last_order_books["bid"].insert(make_pair(iter->first, iter->second));
            }
            else
                break;
        }
        */
    }

}
