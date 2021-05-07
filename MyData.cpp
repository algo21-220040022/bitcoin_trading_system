//
//  data.cpp
//  Backtest Environment
//
//

#include "MyData.hpp"
#include <tuple>
#include <iostream>

using namespace std;

// Gets data from Yahoo Finance CSV's and returns them in format
// An interface to allow for getting "latest"
HistoricalDataHandler::HistoricalDataHandler(boost::ptr_vector<Event>* i_events, vector<string>* i_symbol_list, string* i_start_date, string* i_end_date, int* i_continue_backtest, long _last_period) {

    events = i_events;
    symbol_list = i_symbol_list;
    continue_backtest = i_continue_backtest;
    start_date = i_start_date;
    end_date = i_end_date;
    last_period = _last_period;
}

// Placeholder initializer
HistoricalDataHandler::HistoricalDataHandler() = default;

// Appends unique dates to master dates list
void HistoricalDataHandler::append_to_dates(vector<long> new_dates, string which) {
    if (which == "allDates") {
        if (allDates.empty()) {
            allDates = new_dates;
        }
        else {
            allDates.insert(allDates.end(), new_dates.begin(), new_dates.end());
            sort(allDates.begin(), allDates.end());
            // delete duplicate elems.
            auto last = unique(allDates.begin(), allDates.end());
            allDates.erase(last, allDates.end());
        }
    }
    else if (which == "latestDates") {
        if (latestDates.empty()) {
            latestDates = new_dates;
        }
        else {
            latestDates.insert(latestDates.end(), new_dates.begin(), new_dates.end());
            sort(latestDates.begin(), latestDates.end());
            auto last = unique(latestDates.begin(), latestDates.end());
            latestDates.erase(last, latestDates.end());
        }
    }
}

// Format symbol data

void HistoricalDataHandler::format_data() {
    allDates = {};
    latestDates = {};
    datesbegin = get_epoch_time(*start_date);
    datesend = get_epoch_time(*end_date);

    symbol_tickers = {};
    previous_ticker = {};
    currentDatesIndex = {};

    // Loop through symbols and load their data into a single frame
    for (int i = 0; i < symbol_list->size(); i++) {
        string symbol = (*symbol_list)[i];

        cout << "Loading " << symbol << " data..." << endl;

        // Get the data from Yahoo Finance
        // Access formula is symbol_data[SYMBOL].data[TYPE][DATE]
        MyDataFrame marketData(symbol, last_period);
        marketData.load_data_from_cloud(start_date, end_date);
        symbol_tickers[symbol] = marketData.price_tickers;
        //symbol_order_books[symbol] = marketData.order_books;
        currentDatesIndex[symbol] = 0;
        append_to_dates(marketData.epoch_list, "allDates");

        long last_start_date = get_epoch_time(*start_date) - 86400;
        long last_end_date = get_epoch_time(*start_date) - 1;
        //marketData.load_last_data(&last_start_date, &last_end_date);
        append_to_dates(marketData.last_epoch_list, "latestDates");
    }
}


// Gets input iterator for going through data with forward merge implemented within
/*
map<long, double> HistoricalDataHandler::get_new_ticker(string symbol) {
    // Spits out a bar until there are no more bars to yield
    tuple<string, long, double, double, double, double, double, double> last_ticker;
    long date = allDates[currentDatesIndex[symbol]];

    // Formatted in symbol - date - open - low - high - close - volume
    if (symbol_data[symbol]["open"][date] != 0) {
        lastbar = make_tuple(symbol, date, symbol_data[symbol]["open"][date], symbol_data[symbol]["low"][date], symbol_data[symbol]["high"][date],
            symbol_data[symbol]["close"][date], symbol_data[symbol]["adj"][date], symbol_data[symbol]["volume"][date]);
        previousbar[symbol] = lastbar;
        currentDatesIndex[symbol]++;
    }
    else {
        // If data is not found, use data from last get_new_bar call
        lastbar = previousbar[symbol];
        cout << "Data not found on day " << to_string(date) << " for symbol " << symbol << endl;
    }
    return lastbar;
}
*/

// Get latest N tickers
map<string, map<long, double>> HistoricalDataHandler::get_latest_ticker(string symbol, int N) {

    // If symbol exists in latest_data
    map<string, map<long, double>> ticker_list;

    for (int i = N; i > 0; i--) {
        long date = latestDates[latestDates.size() - i];
        ticker_list["close"][date] = symbol_tickers[symbol]["close"][date];
    }

    // Return cleaned bars list
    return ticker_list;
}
/*
map<string, map<long, map<double, double>>> HistoricalDataHandler::get_last_period_order_books(std::string symbol, int N)
{
    map<string, map<long, map<double, double>>> last_period_order_books;
    long last_end_epoch = latestDates.back();
    int i = N;
    map<double, double > tmp;
    for (auto iter = symbol_order_books[symbol]["ask"].rbegin(); iter != symbol_order_books[symbol]["ask"].rend(); iter++) {
        if (iter->first <= last_end_epoch){
            tmp = iter->second;
            last_period_order_books["ask"][i] = tmp;
            i--;
            if (i <= 0) break;
        }
    }
    i = N;
    for (auto iter = symbol_order_books[symbol]["bid"].rbegin(); iter != symbol_order_books[symbol]["bid"].rend(); iter++) {
        if (iter->first <= last_end_epoch){
            tmp = iter->second;
            last_period_order_books["bid"][i] = tmp;
            i--;
            if (i <= 0) break;
        }
    }
    return last_period_order_books;
}
*/
// Push the latest bar to latest_data for all symbols in list

void HistoricalDataHandler::update_bars() {

    for (int i = 0; i < symbol_list->size(); i++) {
        string symbol = (*symbol_list)[i];
        long new_date = allDates[currentDatesIndex[symbol]];

        previous_ticker[symbol]["close"] = symbol_tickers[symbol]["close"][new_date];
        /*
        for (auto iter = symbol_order_books[symbol]["ask"].end(); iter != symbol_order_books[symbol]["ask"].begin(); iter--)
            if (iter->first <= new_date){
                auto test = iter->second;
                for (auto it = test.begin(); it != test.end(); it++) cout << it->first << " " << it->second << endl;
                previous_order_books[symbol]["ask"] = iter->second;
                break;
            }
        for (auto iter = symbol_order_books[symbol]["bid"].end(); iter != symbol_order_books[symbol]["bid"].begin(); iter--)
            if (iter->first <= new_date){
                previous_order_books[symbol]["bid"] = iter->second;
                break;
            }
        */
        // Add new date to latestDates
        if (latestDates.back() != new_date) {
            latestDates.push_back(new_date);
        }

        // Check if there are any more bars to get
        if (currentDatesIndex[symbol] == allDates.size()) {
            *continue_backtest = 0;
        }
        currentDatesIndex[symbol]++;
    }
    
    // Add bar as a MarketEvent
    events->push_back(new MarketEvent());
}

