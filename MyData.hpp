//
//  data.hpp
//  Backtest Environment
//
#pragma once
#ifndef MyData_hpp
#define MyData_hpp

#ifndef boost
#include <boost/coroutine2/all.hpp>
#endif
#ifndef ptr_vector
#include <boost/ptr_container/ptr_vector.hpp>
#endif

#include <stdio.h>
#include "MyDataFrame.hpp"

#include "events.hpp"

// Abstract base class so cannot be instantiated directly
/*
class DataHandler {
public:
    // Handles data both historical and live
    // Virtual functions to be used by subclasses
    virtual map<string, map<long, double>> get_latest_ticker(std::string symbol, int N = 1);
    virtual map<long, map<double, double>> get_last_period_order_books();
    virtual void update_bars();
};*/

// Historical data handler that puts data onto an Event Queue
class HistoricalDataHandler{
public:
    map<string, map<string, map<long, double>>> symbol_tickers;
   // map<string, map<string, map<long, map<double, double>>>> symbol_order_books;
    map<string, map<string, double>> previous_ticker;
   // map<string, map<string, map<double, double>>> previous_order_books;
    vector<long> allDates;
    vector<long> latestDates;
    map<string, int> currentDatesIndex;
    long last_period;
    int* continue_backtest;
    boost::ptr_vector<Event>* events;
    vector<string>* symbol_list;
    string* start_date;
    string* end_date;
    long datesbegin;
    long datesend;

    // PARAMS:
    // events: Event Queue on which to push new events
    // csv_dir: the path to the folder containing all the symbol csvs
    // symbol_list: list of all the symbols being traded
    HistoricalDataHandler(boost::ptr_vector<Event>* events, vector<string>* symbol_list, string* i_start_date, string* i_end_date, int* i_continue_backtest, long _last_period);

    // Placeholder initializer
    HistoricalDataHandler();

    // Format the symbol data
    void format_data();

    // Creates input iterator for going through data
    map<long, double> get_new_ticker(string symbol);

    // Parent DataHandler functions
    map<string, map<long, double>> get_latest_ticker(std::string symbol, int N = 1);

    //map<string, map<long, map<double, double>>> get_last_period_order_books(std::string symbal, int N=1);

    void update_bars();

    void append_to_dates(vector<long> new_dates, string which);
};

#endif

