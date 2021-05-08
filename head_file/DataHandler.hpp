#pragma once
#ifndef DataHandler_hpp
#define DataHandler_hpp

#include <string>
#include <iostream>
#include <iomanip>
#include <time.h>
#include "Mysqlmgr.hpp"
#include <map>
using namespace std;
#pragma comment(lib,"libmysql.lib")
#pragma comment(lib,"ws2_32.lib")
#define _CRT_SECURE_NO_WARNINGS

// Find epoch time from normal YYYY-MM-DD
extern long get_epoch_time(string date);
// Find normal time YYYY-MM-DD from epoch time
extern string get_std_time(long epochtime);

extern string transform_to_target_date_interval(std::string* start_date, std::string* end_date);
/*
template<class MySQL>
map<long, map<double, double>> load_ask_orders_from_cloud(std::string* start_date, std::string* end_date, MySQL& sqlmgr)
{

	auto time_interval = transform_to_target_date_interval(start_date, end_date);
	const std::string statement = "SELECT * FROM asks WHERE " + time_interval;

	//const std::string statement = "SELECT * FROM asks WHERE time > \"2021-04-30 00:00:00\" and time < \"2021-04-30 00:01:11\"";
	std::cout << "Load ask orders data from cloud SQL....\n";
	Result r = sqlmgr.query(statement);
	if (!r)
	{
		printf("query error %s\n", sqlmgr.geterror());
		map<long, map<double, double>> nothing;
		return nothing;
	}
	map<long, map<double, double>> ask_orders;
	Row row = r.next();
	auto last_timestamp = get_epoch_time(row.columntext(0));
	auto bid_price = stod(row.columntext(1));
	auto bid_volume = stod(row.columntext(2));
	map<double, double> tmp;
	tmp.insert(make_pair(bid_price, bid_volume));

	while (Row row = r.next())
	{
		auto timestamp = get_epoch_time(row.columntext(0));
		bid_price = stod(row.columntext(1));
		bid_volume = stod(row.columntext(2));
		if (timestamp == last_timestamp)
		{
			tmp.insert(make_pair(bid_price, bid_volume));
		}
		else
		{
			ask_orders[last_timestamp] = tmp;
			tmp.clear();
		}
		last_timestamp = timestamp;
	}
	cout << "Finish loading ask orders data!\n";
	return ask_orders;
}
*/
/*
template<class MySQL>
map<long, map<double, double>>  load_bid_orders_from_cloud(std::string* start_date, std::string* end_date, MySQL& sqlmgr)
{

	auto time_interval = transform_to_target_date_interval(start_date, end_date);
	const std::string statement = "SELECT * FROM bids WHERE " + time_interval;

	//const std::string statement = "SELECT * FROM asks WHERE time > \"2021-04-30 00:00:00\" and time < \"2021-04-30 00:01:11\"";
	std::cout << "Load bid orders data from cloud SQL....\n";
	Result r = sqlmgr.query(statement);
	if (!r)
	{
		printf("query error %s\n", sqlmgr.geterror());
		map<long, map<double, double>> nothing;
		return nothing;
	}
	map<long, map<double, double>> bid_orders;
	Row row = r.next();
	auto last_timestamp = get_epoch_time(row.columntext(0));
	auto bid_price = stod(row.columntext(1));
	auto bid_volume = stod(row.columntext(2));
	map<double, double> tmp;
	tmp.insert(make_pair(bid_price, bid_volume));

	while (Row row = r.next())
	{
		auto timestamp = get_epoch_time(row.columntext(0));
		bid_price = stod(row.columntext(1));
		bid_volume = stod(row.columntext(2));
		if (timestamp == last_timestamp)
		{
			tmp.insert(make_pair(bid_price, bid_volume));
		}
		else
		{
			bid_orders[last_timestamp] = tmp;
			tmp.clear();
		}
		last_timestamp = timestamp;
	}
	cout << "Finish loading bid orders data!\n";
	return bid_orders;
}
*/

template<class MySQL>
map<long, double>  load_price_tickers_from_cloud(std::string* start_date, std::string* end_date, MySQL* sqlmgr)
{
	auto time_interval = transform_to_target_date_interval(start_date, end_date);
	const std::string statement = "SELECT time, close FROM priceticker WHERE " + time_interval;

	//const std::string statement = "SELECT * FROM asks WHERE time > \"2021-04-30 00:00:00\" and time < \"2021-04-30 00:01:11\"";
	std::cout << "Load price tickers data from cloud SQL....\n";
	Result r = sqlmgr->query(statement);
	if (!r)
	{
		printf("query error %s\n", sqlmgr->geterror());
		map<long, double> nothing;
		return nothing;
	}
	map<long, double> price_tickers;
	while (Row row = r.next())
	{
		auto timestamp = get_epoch_time(row.columntext(0));
		auto close = stod(row.columntext(1));
		price_tickers[timestamp] = close;
	}
	cout << "Finish loading price tickers data!\n";
	return price_tickers;
}


#endif