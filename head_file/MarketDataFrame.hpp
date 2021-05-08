//
//  MarketDataFrame.hpp
//  Backtest Environment
//
//

#ifndef MarketDataFrame_hpp
#define MarketDataFrame_hpp
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
using namespace std;

class MarketDataFrame {
public:
    map<string, map<long, double>> marketData;
    vector<long> indices;
    string symbol;
    MarketDataFrame(string symbl);
    MarketDataFrame();
    void read_data_from_csv(char csv_file[FILENAME_MAX]);

};

long get_epoch_time(string date);
string get_std_time(long epochtime);

#endif /* MarketDataFrame_hpp */
