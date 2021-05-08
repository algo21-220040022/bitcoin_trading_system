//
//  execution.hpp
//  Backtest Environment
//
//
#pragma once
#ifndef Execution_hpp
#define Execution_hpp
#ifndef vector
#include <vector>
#endif
#ifndef ptr_vector
#include <boost/ptr_container/ptr_vector.hpp>
#endif
/*
#ifndef chrono
#include <chrono>
#endif
*/
#ifndef Event
#include "Events.hpp"
#endif
#ifndef HistoricalDataHandler
#include "MyData.hpp"
#endif

// Base execution handler to which we can add features
class ExecutionHandler {
public:
    boost::ptr_vector<Event>* eventlist;
    HistoricalDataHandler* data;
    virtual void execute_order(OrderEvent event) = 0;
};

// Simulated execution handler that assumes all orders are filled at the current market price
// for all quantities; need more sophisticated slippage and market impact
class SimulatedExecutionHandler : ExecutionHandler {
public:
    // Constructor
    SimulatedExecutionHandler(boost::ptr_vector<Event>* events, HistoricalDataHandler* data);

    // Placeholder constructor
    SimulatedExecutionHandler() = default;

    // Inherited order execution system
    void execute_order(OrderEvent event);

    // Slippage calculations
    std::pair<double, double> calculate_slippage(OrderEvent event);
};

#endif /* execution_hpp */
