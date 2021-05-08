//
//  benchmark.cpp
//  Backtest Environment
//

#include "MyStrategy.hpp"

#ifndef ptr_vector
#include <boost/ptr_container/ptr_vector.hpp>
#endif

// Initialize strategy
Benchmark::Benchmark(HistoricalDataHandler* i_data, boost::ptr_vector<Event>* i_events) {

    // Set instance variables
    data = i_data;
    events = i_events;
    symbol_list = data->symbol_list;

    // Create the bought dictionary
    bought = calculate_initial_bought();
}

// Placeholder initializer
Benchmark::Benchmark() = default;

// Create map of bought symbols (all empty)
map<string, bool> Benchmark::calculate_initial_bought() {
    bought = {};
    for (int i = 0; i < symbol_list->size(); i++) {
        bought[(*symbol_list)[i]] = false;
    }
    return bought;
}

// Update map of bought symbols
void Benchmark::calculate_signals(MarketEvent i_event) {

    // Go LONG in SPY at first MarketEvent, as it will be the only symbol in the symbol list
    for (int i = 0; i < symbol_list->size(); i++) {
        string symbol = (*symbol_list)[i];
        map<string, map<long, double>> new_tickers = data->get_latest_ticker(symbol, 1);
        double price = new_tickers["close"].begin().operator*().second;
        cout << "Current market price of BTC:" << price << endl;
        if (!new_tickers.empty() && price > 0.01) {
            if (!bought[symbol]) {
                // (symbol, time, type=LONG, SHORT or EXIT)
                events->push_back(new SignalEvent(symbol, price, 0.5, "BENCH"));
                bought[symbol] = true;
            }
        }
    }
}
