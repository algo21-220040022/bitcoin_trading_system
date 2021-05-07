#pragma once
//
//  strategiesheader.hpp
//  Backtest Environment
//

#ifndef MyStrategy_hpp
#define MyStrategy_hpp
#ifndef DataHandler
#include "MyData.hpp"
#endif
#ifndef Event
#include "events.hpp"
#endif

#include <stdio.h>

using namespace std;

// Strategy class to be inherited by all strategies
class Strategy {
public:
    // Function that sends signals based on market data
    virtual void calculate_signals(MarketEvent event) = 0;
};

// Benchmark class that goes 100% long in bitcoin on the first MarketEvent and holds forever
// Mimics the setup of a real strategy to allow for easy editing
class Benchmark : Strategy {
public:
    HistoricalDataHandler* data;
    boost::ptr_vector<Event>* events;
    vector<string>* symbol_list;
    map<string, bool> bought;

    Benchmark();
    // Initialize instance of Buy and Hold strategy
    Benchmark(HistoricalDataHandler* i_data, boost::ptr_vector<Event>* i_events);
    // Add keys for all symbols in symbol_list to bought and sets them to false
    map<string, bool> calculate_initial_bought();
    // Trading logic in this function for new event
    void calculate_signals(MarketEvent event);
};


class MainStrategy : Strategy {
public:
    // Default strategy context variables
    HistoricalDataHandler* data;
    boost::ptr_vector<Event>* events;
    vector<string>* symbol_list;
    string title;

    // Custom strategy variables
    int lookback = 20;                  // Lookback for regression for hedge calculation
    int z_lookback = 20;                // Lookback for Z-score calculation, must be <= lookback
    vector<double> spreads = {};        // Vector holding the spreads between the pair
    bool inLong = false;                // True when long in the pair
    bool inShort = false;               // True when short in the pair

    MainStrategy();
    // Initialize instance of Main Strategy
    MainStrategy(HistoricalDataHandler* i_data, boost::ptr_vector<Event>* i_events);
    // Trading logic in this function for new event
    void calculate_signals(MarketEvent event);


    // Custom strategy functions

    map<string, bool> bought;
    map<string, bool> calculate_initial_bought();

    /*
        double calculate_hedge_ratio(vector<double>X, vector<double>Y);
        vector<double> computeHoldingsPct(double yShares, double xShares, double yPrice, double xPrice);
    */
};

#endif /* strategiesheader_hpp */