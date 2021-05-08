//
//  events.cpp
//  Backtest Environment
//
//

#include "events.hpp"

using namespace std;

// Market event initialization
MarketEvent::MarketEvent() {
    type = "MARKET";
}

// Signal event initialization
SignalEvent::SignalEvent(string i_symbol, long i_datetime, double percentage, string i_target) {
    type = "SIGNAL";
    symbol = i_symbol;
    datetime = i_datetime;
    strength = percentage;
    target = i_target;
}

// Order event initialization
OrderEvent::OrderEvent(string i_symbol, string i_order_type, double i_quantity, string i_direction, string i_target) {
    type = "ORDER";
    symbol = i_symbol;
    order_type = i_order_type;
    quantity = i_quantity;
    direction = i_direction;
    target = i_target;
}

// Default order event initializer
OrderEvent::OrderEvent() { }

// Fill event initialization
FillEvent::FillEvent(long i_timeindex, string i_symbol, string i_exchange, double i_quantity, string i_direction, double i_fill_cost, double i_commission, string i_target) {
    type = "FILL";
    timeindex = i_timeindex;
    symbol = i_symbol;
    exchange = i_exchange;
    quantity = i_quantity;
    direction = i_direction;
    slippage = i_fill_cost;
    target = i_target;

    // Calculate commission
    if (i_commission >= 0) {
        commission = i_commission;
    }
    else {
        commission = calculate_IB_commission();
    }
}

// Calculate commission based on Interactive Brokers' rates (outdated at the moment)
double FillEvent::calculate_IB_commission() {
    double min_cost = 6;
    double commission_rate = 0.0025;
    double full_cost = max(min_cost, commission_rate * quantity);
    return full_cost;
}
