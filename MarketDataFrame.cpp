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
#include "MarketDataFrame.hpp"
#include <time.h>

map<string, map<long, double>> data;
vector<long> indices;

// Find epoch time from normal YYYY-MM-DD
long get_epoch_time(string date) {
    string delimiter = "-";
    string token;
    struct tm t = { 0 };
    t.tm_year = stoi(date.substr(0, 4));
    t.tm_mon = stoi(date.substr(5, 7));
    t.tm_mday = stoi(date.substr(8, 10));
    t.tm_hour = stoi(date.substr(11, 13));
    t.tm_min = stoi(date.substr(14, 16));
    t.tm_sec = stoi(date.substr(17, 19));
    

    // Get time since 1900 epoch
    t.tm_year -= 1900;
    t.tm_mon--;

    time_t timeSinceEpoch = mktime(&t);

    return long(timeSinceEpoch);
}

// Find normal time YYYY-MM-DD from epoch time
string get_std_time(long epochtime) {
    time_t time = epochtime;
    tm* tm_ = localtime(&time);                
    int year, month, day, hour, minute, second;
    year = tm_->tm_year + 1900;                
    month = tm_->tm_mon + 1;                   
    day = tm_->tm_mday;                        
    hour = tm_->tm_hour;                     
    minute = tm_->tm_min;                     
    second = tm_->tm_sec;                   
    char yearStr[5], monthStr[3], dayStr[3], hourStr[3], minuteStr[3], secondStr[3];
    sprintf(yearStr, "%d", year);           
    sprintf(monthStr, "%d", month);      
    sprintf(dayStr, "%d", day);             
    sprintf(hourStr, "%d", hour);          
    sprintf(minuteStr, "%d", minute);
    
    // Transform the hour, minute, second to 2-bits
    if (hourStr[1] == '\0')
    {
        hourStr[2] = '\0';
        hourStr[1] = hourStr[0];
        hourStr[0] = '0';
    }
    if (minuteStr[1] == '\0')                 
    {
        minuteStr[2] = '\0';
        minuteStr[1] = minuteStr[0];
        minuteStr[0] = '0';
    }
    sprintf(secondStr, "%d", second);         
    if (secondStr[1] == '\0')                  
    {
        secondStr[2] = '\0';
        secondStr[1] = secondStr[0];
        secondStr[0] = '0';
    }
    char s[20];                                // 定义总日期时间char*变量。
    sprintf(s, "%s-%02s-%02s %02s:%02s:%02s", yearStr, monthStr, dayStr, hourStr, minuteStr, secondStr);// 将年月日时分秒合并。
    //sprintf(s, "%02s-%02s-%02s", yearStr, monthStr, dayStr);
    if (s[5] == ' ') { s[5] = '0'; }
    if (s[8] == ' ') { s[8] = '0'; }
    string str(s);                             // 定义string变量，并将总日期时间char*变量作为构造函数的参数传入。
    return str;                                // 返回转换日期时间后的string变量。
}

// Placeholder default constructor
MarketDataFrame::MarketDataFrame() {}

// Constructor
MarketDataFrame::MarketDataFrame(string symbl) : symbol(symbl) {}

// Takes .csv data in date-OHLCAV format and makes it into a map of maps
void MarketDataFrame::read_data_from_csv(char csv_file[FILENAME_MAX]) {
    cout << csv_file << endl;
    ifstream csv(csv_file);
    string line;
    map<long, double> open;
    map<long, double> high;
    map<long, double> low;
    map<long, double> close;
    map<long, double> adj;
    map<long, double> volume;

    // Iterate through the csv file
    while (getline(csv, line)) {
        string date;

        // Get string stream
        replace(line.begin(), line.end(), ',', ' ');
        stringstream ss(line);

        // Get each datapoint
        //map<string, double> dataRow;
        ss >> date;
        long epochdate = get_epoch_time((char*)date.c_str());
        ss >> open[epochdate];
        ss >> high[epochdate];
        ss >> low[epochdate];
        ss >> close[epochdate];
        ss >> adj[epochdate];
        ss >> volume[epochdate];

        // Add the date to the indices
        indices.push_back(epochdate);
    }

    // Enter data into the mock DataFrame
    marketData["open"] = open;
    marketData["high"] = high;
    marketData["low"] = low;
    marketData["close"] = close;
    marketData["adj"] = adj;
    marketData["volume"] = volume;

    // Pop out the index that says "Date"
    indices.erase(indices.begin());
}
