//
//  strat_buyandhold.cpp
//  Backtest Environment
//

#include "MyStrategy.hpp"
#ifndef sqrt
#include <cmath>
#endif
#ifndef ptr_vector
#include <boost/ptr_container/ptr_vector.hpp>
#endif

// Initialize strategy
MainStrategy::MainStrategy(HistoricalDataHandler* i_data, boost::ptr_vector<Event>* i_events) {

    // Set instance variables
    data = i_data;
    events = i_events;
    *data->symbol_list = { string("BTC") };
    symbol_list = data->symbol_list;

    // Set custom strategy variables
    // Need to instantiate map bought in header file
    bought = calculate_initial_bought();
}

// Placeholder initializer
MainStrategy::MainStrategy() = default;

// Create map of bought symbols
map<string, bool> MainStrategy::calculate_initial_bought() {
    bought = {};
    for (int i = 0; i < symbol_list->size(); i++) {
        bought[(*symbol_list)[i]] = false;
    }
    return bought;
}

// return: "up" or "down"; degree


map<string, int> cal_signal(map<string, map<long, double>>& _new_tickers)
{
    map<string, int> signal;
    int count = 0;
    double trade_price = _new_tickers["close"].begin().operator*().second;
    double sum_price = 0;
    int period = 20;
    int period_s = 10;
    double ave_price_s = 0;
    int i = 0;
    for (auto iter = _new_tickers["close"].rbegin(); iter != _new_tickers["close"].rend()&&i<period; iter++,i++)
    {
        sum_price += iter->second;
        if (i == period_s - 1)
            ave_price_s = sum_price/period_s;
    }
    double ave_price = sum_price / period;
    double total_sum_square = 0;
    i = 0;
    for (auto iter = _new_tickers["close"].rbegin(); iter != _new_tickers["close"].rend() && i < period; iter++,i++)
    {
        double tmp_price = iter->second;
        total_sum_square += (tmp_price - ave_price)*(tmp_price - ave_price);
    }
    double std = sqrt(total_sum_square/period);
    double up_BB = ave_price + 2.0 * std;
    double down_BB = ave_price - 2.0 * std;
    cout << "Average price=" << ave_price << "; up_BB=" << up_BB << "; std=" << std << "\n";
    if (ave_price_s > ave_price && trade_price < up_BB)
        signal["direction"] = 1;
    else if (ave_price_s < ave_price - 0.01*std)
        signal["direction"] = -1;
    else
        signal["direction"] = 2;
    /*
    if (trade_price < down_BB)
        signal["direction"] = 1;
    else if (trade_price > up_BB)
        signal["direction"] = -1;
    else
        signal["direction"] = 0;
    */
    return signal;
}


// Update map of bought symbols
void MainStrategy::calculate_signals(MarketEvent i_event) {

    for (int i = 0; i < symbol_list->size(); i++) {
        string symbol = "BTC";
        map<string, map<long, double>> new_tickers = data->get_latest_ticker(symbol, 30);
        //map<string, map<long, map<double, double>>> new_order_books =  data->get_last_period_order_books(symbol, 1);
        map<string, int> signal = cal_signal(new_tickers);
        if (signal["direction"] == 1 && !bought[symbol]) {
            events->push_back(new SignalEvent(symbol, new_tickers["close"].begin().operator*().second, 0.5));
            bought[symbol] = true;
        }
        else if ((signal["direction"] == -1 && bought[symbol]) || (signal["direction"] == 0 && bought[symbol])) {
            events->push_back(new SignalEvent(symbol, new_tickers["close"].begin().operator*().second, 0));
            bought[symbol] = false;
        }

    }
}
