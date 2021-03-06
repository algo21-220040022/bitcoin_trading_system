#pragma once
//
//  interface.hpp
//  Backtest Environment
//
//

#ifndef Interface_hpp
#define Interface_hpp
#ifndef string
#include <string>
#endif
#ifndef vector
#include <vector>
#endif
#ifndef iostream
#include <iostream>
#endif
#include "Portfolio.hpp"

#include "MyStrategy.hpp"
#include "Execution.hpp"

#include <boost/ptr_container/ptr_vector.hpp>

// Create a class for different types of trading interfaces
class TradingInterface {
public:

    // VARIABLES
    // executor: logic for executing trades (acts as a broker, essentially)
    // portfolio: portfolio that converts orders into fills and sets rules
    // pipeline: Data Handler instantiated from data.cpp
    // events: global list of events

    vector<string>* symbol_list;
    vector<string> benchmarksymbols;
    double* initial_capital;
    int continue_backtest;
    string* startdate;
    string* enddate;

    // Global events and execution handler
    boost::ptr_vector<Event> events;
    SimulatedExecutionHandler executor;

    // Strategy setup
    NaivePortfolio portfolio;
    HistoricalDataHandler pipeline;
    MainStrategy strat;

    // Benchmark setup
    HistoricalDataHandler& benchmarkpipeline= pipeline;
    NaivePortfolio benchmarkportfolio;
    Benchmark benchmark;

    // PARAMS
    // symbol_list: list of symbols to be traded
    // initial_cap: initial amount of capital allocated to algorithm
    // strategy: the algorithm whose signals are sent to the portfolio
    TradingInterface(vector<string>* symbol_list, vector<string>benchmarksymbols, double* initial_cap, string* start_date, string* end_date, long _last_period);
    // Placeholder initializer
    TradingInterface() = default;

    // Executes the while loop for running the backtest
    void runbacktest(MainStrategy& strategy, Benchmark& benchmark);
    void runlivetrade(MainStrategy& strategy, Benchmark& i_benchmark);

};

#endif /* interface_hpp */