#include "Interface.hpp"
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/strand.hpp>
#include <boost/make_unique.hpp>
#include <boost/optional.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/bind/bind.hpp>
#include <boost/asio/placeholders.hpp>

#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <regex>
#include<windows.h>
#include<boost/date_time/posix_time/posix_time.hpp>


namespace beast = boost::beast;                 // from <boost/beast.hpp>
namespace http = beast::http;                   // from <boost/beast/http.hpp>
namespace websocket = beast::websocket;         // from <boost/beast/websocket.hpp>
namespace net = boost::asio;                    // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;               // from <boost/asio/ip/tcp.hpp>

// Return a reasonable mime type based on the extension of a file.
beast::string_view
mime_type(beast::string_view path)
{
    using beast::iequals;
    auto const ext = [&path]
    {
        auto const pos = path.rfind(".");
        if (pos == beast::string_view::npos)
            return beast::string_view{};
        return path.substr(pos);
    }();
    if (iequals(ext, ".htm"))  return "text/html";
    if (iequals(ext, ".html")) return "text/html";
    if (iequals(ext, ".php"))  return "text/html";
    if (iequals(ext, ".css"))  return "text/css";
    if (iequals(ext, ".txt"))  return "text/plain";
    if (iequals(ext, ".js"))   return "application/javascript";
    if (iequals(ext, ".json")) return "application/json";
    if (iequals(ext, ".xml"))  return "application/xml";
    if (iequals(ext, ".swf"))  return "application/x-shockwave-flash";
    if (iequals(ext, ".flv"))  return "video/x-flv";
    if (iequals(ext, ".png"))  return "image/png";
    if (iequals(ext, ".jpe"))  return "image/jpeg";
    if (iequals(ext, ".jpeg")) return "image/jpeg";
    if (iequals(ext, ".jpg"))  return "image/jpeg";
    if (iequals(ext, ".gif"))  return "image/gif";
    if (iequals(ext, ".bmp"))  return "image/bmp";
    if (iequals(ext, ".ico"))  return "image/vnd.microsoft.icon";
    if (iequals(ext, ".tiff")) return "image/tiff";
    if (iequals(ext, ".tif"))  return "image/tiff";
    if (iequals(ext, ".svg"))  return "image/svg+xml";
    if (iequals(ext, ".svgz")) return "image/svg+xml";
    return "application/text";
}

// Append an HTTP rel-path to a local filesystem path.
// The returned path is normalized for the platform.
std::string
path_cat(
    beast::string_view base,
    beast::string_view path)
{
    if (base.empty())
        return std::string(path);
    std::string result(base);
#ifdef BOOST_MSVC
    char constexpr path_separator = '\\';
    if (result.back() == path_separator)
        result.resize(result.size() - 1);
    result.append(path.data(), path.size());
    for (auto& c : result)
        if (c == '/')
            c = path_separator;
#else
    char constexpr path_separator = '/';
    if (result.back() == path_separator)
        result.resize(result.size() - 1);
    result.append(path.data(), path.size());
#endif
    return result;
}

// This function produces an HTTP response for the given
// request. The type of the response object depends on the
// contents of the request, so the interface requires the
// caller to pass a generic lambda for receiving the response.
template<class Body, class Allocator, class Send>
void handle_request(beast::string_view doc_root, http::request<Body,
    http::basic_fields<Allocator>>&& req, Send&& send)
{
    // Returns a bad request response
    auto const bad_request =
        [&req](beast::string_view why)
    {
        http::response<http::string_body> res{ http::status::bad_request, req.version() };
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.body() = std::string(why);
        res.prepare_payload();
        return res;
    };

    // Returns a not found response
    auto const not_found =
        [&req](beast::string_view target)
    {
        http::response<http::string_body> res{ http::status::not_found, req.version() };
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.body() = "The resource '" + std::string(target) + "' was not found.";
        res.prepare_payload();
        return res;
    };

    // Returns a server error response
    auto const server_error =
        [&req](beast::string_view what)
    {
        http::response<http::string_body> res{ http::status::internal_server_error, req.version() };
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.body() = "An error occurred: '" + std::string(what) + "'";
        res.prepare_payload();
        return res;
    };

    // Make sure we can handle the method
    if (req.method() != http::verb::get &&
        req.method() != http::verb::head)
        return send(bad_request("Unknown HTTP-method"));

    // Request path must be absolute and not contain "..".
    if (req.target().empty() ||
        req.target()[0] != '/' ||
        req.target().find("..") != beast::string_view::npos)
        return send(bad_request("Illegal request-target"));

    // Build the path to the requested file
    std::string path = path_cat(doc_root, req.target());
    if (req.target().back() == '/')
        path.append("index.html");

    // Attempt to open the file
    beast::error_code ec;
    http::file_body::value_type body;
    body.open(path.c_str(), beast::file_mode::scan, ec);

    // Handle the case where the file doesn't exist
    if (ec == beast::errc::no_such_file_or_directory)
        return send(not_found(req.target()));

    // Handle an unknown error
    if (ec)
        return send(server_error(ec.message()));

    // Cache the size since we need it after the move
    auto const size = body.size();

    // Respond to HEAD request
    if (req.method() == http::verb::head)
    {
        http::response<http::empty_body> res{ http::status::ok, req.version() };
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, mime_type(path));
        res.content_length(size);
        res.keep_alive(req.keep_alive());
        return send(std::move(res));
    }

    // Respond to GET request
    http::response<http::file_body> res{
        std::piecewise_construct,
        std::make_tuple(std::move(body)),
        std::make_tuple(http::status::ok, req.version()) };
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, mime_type(path));
    res.content_length(size);
    res.keep_alive(req.keep_alive());
    return send(std::move(res));
}

//------------------------------------------------------------------------------
//--------------------Backtest and live func-------------------------------------
string backtest(string* start_date_ptr, string* end_date_ptr, string type)
{
    vector<string> symbol_list{ "BTC" };
    vector<string> benchmarksymbols{ "BTC" };
    double initial_cap(1000000);
    //string start_date("2021-05-03 10:00:00");
    //string end_date("2021-05-03 21:00:00");

    vector<string>* symbol_list_ptr = &symbol_list;
    double* initial_cap_ptr = &initial_cap;
    //string* start_date_ptr = &start_date;
    //string* end_date_ptr = &end_date;
    long _last_period = 30;
    TradingInterface myTradingInterface(symbol_list_ptr, benchmarksymbols, initial_cap_ptr, start_date_ptr, end_date_ptr, _last_period);

    MainStrategy strategy(&myTradingInterface.pipeline, &myTradingInterface.events);
    Benchmark i_benchmark(&myTradingInterface.benchmarkpipeline, &myTradingInterface.events);

    myTradingInterface.runbacktest(strategy, i_benchmark);

    // Transform the backtest result to json form
    string comma = ",";
    string right_blank("]");
    string right_flower("}");
    string double_quote("\"");
    
    string result("{\"type\":");
    result += type + comma;
    map<long, map<string, double>>* my_portfolio = &myTradingInterface.portfolio.all_holdings;
    map<string, double>* my_current_holding = &myTradingInterface.portfolio.current_holdings;
    map<long, map<string, double>>* benchmark_portfolio = &myTradingInterface.benchmarkportfolio.all_holdings;
    string date("\"date\":[");
    string my_net_value("\"net_value\":[");
    string total_return("\"total_return\":");
    string total_return_tmp = to_string(my_portfolio->rbegin().operator*().second["equitycurve"] * 100);
    total_return += total_return_tmp;
    string max_dd("\"max_dd\":");
    string max_dd_tmp = to_string(my_portfolio->rbegin().operator*().second["drawdown"] * 100);
    max_dd += max_dd_tmp;
    string volatility("\"volatility\":");
    string volatility_tmp = to_string(my_portfolio->rbegin().operator*().second["variance"] * 100 * 100);
    volatility += volatility_tmp;
    string sharpe("\"sharpe\":");
    string sharpe_tmp = to_string(my_portfolio->rbegin().operator*().second["sharpe"]);
    sharpe += sharpe_tmp;
    string mean_ret("\"mean_ret\":");
    string mean_ret_tmp = to_string(my_portfolio->rbegin().operator*().second["mean"]);
    mean_ret += mean_ret_tmp;
    string pnl("\"pnl\":");
    string pnl_tmp = to_string(my_portfolio->rbegin().operator*().second["totalholdings"] - initial_cap);
    pnl += pnl_tmp;
    string position("\"position\":");
    string position_tmp = to_string((*my_current_holding)[symbol_list[0]]);
    position += position_tmp;
    string cash("\"cash\":");
    string cash_tmp = to_string(my_portfolio->rbegin().operator*().second["totalholdings"] - (*my_current_holding)[symbol_list[0]]);
    cash += cash_tmp;

    string bench_net_value("\"bench_net_value\":[");
    string bench_total_return("\"bench_total_return\":");
    string bench_total_return_tmp = to_string(benchmark_portfolio->rbegin().operator*().second["equitycurve"] * 100);
    bench_total_return += bench_total_return_tmp;
    
    int i = 0;
    int size = my_portfolio->size();
    for (auto iter = my_portfolio->begin(); iter != my_portfolio->end(); iter++) {
        string date_tmp = get_std_time(iter->first);
        string net_value_tmp = to_string(iter->second["totalholdings"]);
        date += double_quote + date_tmp + double_quote;
        my_net_value += net_value_tmp;
        if (i < size - 1) {
            date += comma;
            my_net_value += comma;
        }
        else {
            date += right_blank;
            my_net_value += right_blank;
        }
        i++;
    }
    i = 0;
    size = benchmark_portfolio->size();
    for (auto iter = benchmark_portfolio->begin(); iter != benchmark_portfolio->end(); iter++) {
        string bench_net_value_tmp = to_string(iter->second["totalholdings"]);
        bench_net_value += bench_net_value_tmp;
        if (i < size - 1) {
            bench_net_value += comma;
        }
        else {
            bench_net_value += right_blank;
        }
        i++;
    }
    result += date + comma + my_net_value + comma + total_return + comma + max_dd + comma + pnl + comma + position + comma + cash + comma +
        volatility + comma + mean_ret + comma + sharpe + comma + bench_net_value + comma + bench_total_return + right_flower;
    return result;
}

NaivePortfolio i_portfolio;
NaivePortfolio i_benchmarkportfolio;
MainStrategy i_strategy;
Benchmark i_benchmark;
bool first_time = true;
long last_end = 0;
double initial_cap = 1000000;
map<long, map<string, double>> my_all_positions;
map<string, double> my_current_positions;
map<long, map<string, double>> my_all_holdings;
vector<double> my_returns_stream;
map<string, double> my_current_holdings;
bool my_bought = false;
map<long, map<string, double>> bench_all_positions;
map<string, double> bench_current_positions;
map<long, map<string, double>> bench_all_holdings;
vector<double> bench_returns_stream;
map<string, double> bench_current_holdings;
bool bench_bought = false;
map<string, double> my_perfmap;
map<string, double> bench_perfmap;
string livetrade(string* start_date_ptr, string* end_date_ptr, string type)
{
    
    vector<string> symbol_list{ "BTC" };
    vector<string> benchmarksymbols{ "BTC" };
    //string start_date("2021-05-03 10:00:00");
    //string end_date("2021-05-03 21:00:00");

    vector<string>* symbol_list_ptr = &symbol_list;
    double* initial_cap_ptr = &initial_cap;
    //string* start_date_ptr = &start_date;
    //string* end_date_ptr = &end_date;
    long _last_period = 30;
    TradingInterface myTradingInterface(symbol_list_ptr, benchmarksymbols, initial_cap_ptr, start_date_ptr, end_date_ptr, _last_period);
    MainStrategy strategy(&myTradingInterface.pipeline, &myTradingInterface.events);
    Benchmark benchmark(&myTradingInterface.benchmarkpipeline, &myTradingInterface.events);
    
    if (first_time) {
        
        myTradingInterface.runbacktest(strategy, benchmark);
        first_time = false;
    }
    else {
        myTradingInterface.portfolio.all_positions = my_all_positions;
        myTradingInterface.portfolio.current_positions = my_current_positions;
        myTradingInterface.portfolio.all_holdings = my_all_holdings;

        myTradingInterface.portfolio.returns_stream = my_returns_stream;
        myTradingInterface.portfolio.current_holdings = my_current_holdings;
        strategy.bought[symbol_list[0]] = my_bought;
        myTradingInterface.benchmarkportfolio.all_positions = bench_all_positions;
        myTradingInterface.benchmarkportfolio.current_positions = bench_current_positions;
        myTradingInterface.benchmarkportfolio.all_holdings = bench_all_holdings;
        myTradingInterface.benchmarkportfolio.returns_stream = bench_returns_stream;
        myTradingInterface.benchmarkportfolio.current_holdings = bench_current_holdings;
        benchmark.bought[symbol_list[0]] = bench_bought;
        myTradingInterface.portfolio.perfmap = my_perfmap;
        myTradingInterface.benchmarkportfolio.perfmap = bench_perfmap;
        myTradingInterface.runlivetrade(strategy, benchmark);
    }
    my_all_positions = myTradingInterface.portfolio.all_positions;
    my_current_positions = myTradingInterface.portfolio.current_positions;
    my_all_holdings = myTradingInterface.portfolio.all_holdings;
    my_returns_stream = myTradingInterface.portfolio.returns_stream;
    my_current_holdings = myTradingInterface.portfolio.current_holdings;
    my_bought = strategy.bought[symbol_list[0]];
    bench_all_positions = myTradingInterface.benchmarkportfolio.all_positions;
     bench_current_positions = myTradingInterface.benchmarkportfolio.current_positions;
    bench_all_holdings = myTradingInterface.benchmarkportfolio.all_holdings;
    bench_returns_stream = myTradingInterface.benchmarkportfolio.returns_stream;
    bench_current_holdings = myTradingInterface.benchmarkportfolio.current_holdings;
    bench_bought = benchmark.bought[symbol_list[0]];
    my_perfmap = myTradingInterface.portfolio.perfmap;
    bench_perfmap = myTradingInterface.benchmarkportfolio.perfmap;
    //initial_cap = myTradingInterface;
    // Transform the backtest result to json form
    string comma = ",";
    string right_blank("]");
    string right_flower("}");
    string double_quote("\"");

    string result("{\"type\":");
    result += type + comma;
    map<long, map<string, double>>* my_portfolio = &myTradingInterface.portfolio.all_holdings;
    map<string, double>* my_current_holding = &myTradingInterface.portfolio.current_holdings;
    map<long, map<string, double>>* benchmark_portfolio = &myTradingInterface.benchmarkportfolio.all_holdings;
    string date("\"date\":[");
    string my_net_value("\"net_value\":[");
    string total_return("\"total_return\":");
    string total_return_tmp = to_string(my_portfolio->rbegin().operator*().second["equitycurve"] * 100);
    total_return += total_return_tmp;
    string max_dd("\"max_dd\":");
    string max_dd_tmp = to_string(my_portfolio->rbegin().operator*().second["drawdown"] * 100);
    max_dd += max_dd_tmp;
    string volatility("\"volatility\":");
    string volatility_tmp = to_string(my_portfolio->rbegin().operator*().second["variance"] * 100 * 100);
    volatility += volatility_tmp;
    string sharpe("\"sharpe\":");
    string sharpe_tmp = to_string(my_portfolio->rbegin().operator*().second["sharpe"]);
    sharpe += sharpe_tmp;
    string mean_ret("\"mean_ret\":");
    string mean_ret_tmp = to_string(my_portfolio->rbegin().operator*().second["mean"]);
    mean_ret += mean_ret_tmp;
    string pnl("\"pnl\":");
    string pnl_tmp = to_string(my_portfolio->rbegin().operator*().second["totalholdings"] - initial_cap);
    pnl += pnl_tmp;
    string position("\"position\":");
    string position_tmp = to_string((*my_current_holding)[symbol_list[0]]);
    position += position_tmp;
    string cash("\"cash\":");
    string cash_tmp = to_string(my_portfolio->rbegin().operator*().second["totalholdings"] - (*my_current_holding)[symbol_list[0]]);
    cash += cash_tmp;

    string bench_net_value("\"bench_net_value\":[");
    string bench_total_return("\"bench_total_return\":");
    string bench_total_return_tmp = to_string(benchmark_portfolio->rbegin().operator*().second["equitycurve"] * 100);
    bench_total_return += bench_total_return_tmp;

    int i = 0;
    int size = my_portfolio->size();
    for (auto iter = my_portfolio->lower_bound(last_end); iter != my_portfolio->end(); iter++) {
        if (i == 0) {
            i++;
            continue;
        }
        string date_tmp = get_std_time(iter->first);
        string net_value_tmp = to_string(iter->second["totalholdings"]);
        date += double_quote + date_tmp + double_quote;
        my_net_value += net_value_tmp;
        date += comma;
        my_net_value += comma;  
        i++;
    }
    
    i = 0;
    size = benchmark_portfolio->size();
    for (auto iter = benchmark_portfolio->lower_bound(last_end); iter != benchmark_portfolio->end(); iter++) {
        if (i == 0) {
            i++;
            continue;
        }
        string bench_net_value_tmp = to_string(iter->second["totalholdings"]);
        bench_net_value += bench_net_value_tmp;
        bench_net_value += comma;

        i++;
    }
    char left_blank = '[';
    if (date[date.length() - 1] != left_blank) {
        date.erase(date.end() - 1);
        my_net_value.erase(my_net_value.end() - 1);
        bench_net_value.erase(bench_net_value.end() - 1);
    }
    date += right_blank;
    my_net_value += right_blank;  
    bench_net_value += right_blank;
    last_end = my_portfolio->rbegin().operator*().first;
    result += date + comma + my_net_value + comma + total_return + comma + max_dd + comma + pnl + comma + position + comma + cash + comma +
        volatility + comma + mean_ret + comma + sharpe + comma + bench_net_value + comma + bench_total_return + right_flower;
    return result;
}





// Report a failure
void fail(beast::error_code ec, char const* what)
{
    std::cerr << what << ": " << ec.message() << "\n";
}

// Echoes back all received WebSocket messages
class websocket_session : public std::enable_shared_from_this<websocket_session>
{
    websocket::stream<beast::tcp_stream> ws_;
    beast::flat_buffer buffer_;
    
    // Timekeeper for doing writing action every X seconds --Added by Kuant 2021/4/4


public:
    string live_start_date = "2021-05-08 11:10:00";
    int time_keeper = 0;
    // Take ownership of the socket
    explicit websocket_session(tcp::socket&& socket) : ws_(std::move(socket))
    {
    }

    // Start the asynchronous accept operation
    template<class Body, class Allocator>
    void do_accept(http::request<Body, http::basic_fields<Allocator>> req)
    {
        // Set suggested timeout settings for the websocket
        ws_.set_option(
            websocket::stream_base::timeout::suggested(
                beast::role_type::server));

        // Set a decorator to change the Server of the handshake
        ws_.set_option(websocket::stream_base::decorator(
            [](websocket::response_type& res)
            {
                res.set(http::field::server,
                    std::string(BOOST_BEAST_VERSION_STRING) +
                    " advanced-server");
            }));

        // Accept the websocket handshake
        ws_.async_accept(
            req,
            beast::bind_front_handler(
                &websocket_session::on_accept,
                shared_from_this()));
    }

private:
    

    void on_accept(beast::error_code ec)
    {
        if (ec)
            return fail(ec, "accept");

        // Read a message
        do_read();
        //do_write();
    }

    void do_read()
    {
        // Read a message into our buffer
        ws_.async_read(
            buffer_,
            beast::bind_front_handler(
                &websocket_session::on_read,
                shared_from_this()));
        std::cout << "reach do_read!\n";
        std::cout << beast::buffers_to_string(buffer_.data());
        //Sleep(3000);
        do_write();
    }

    void on_read(beast::error_code ec, std::size_t bytes_transferred)
    {
        std::cout << "reach here\n";
        boost::ignore_unused(bytes_transferred);

        // This indicates that the websocket_session was closed
        if (ec == websocket::error::closed)
            return;

        if (ec)
            fail(ec, "read");
        std::cout << "reach on_read!\n";
        std::string received_message = beast::buffers_to_string(buffer_.data());
        std::cout << received_message << std::endl;
        respond_the_read_message(received_message);
        // Echo the message
        /*
        ws_.text(ws_.got_text());
        ws_.async_write(
            buffer_.data(),
            beast::bind_front_handler(
                &websocket_session::on_write,
                shared_from_this()));
        */
        buffer_.consume(buffer_.size());
        
        do_read();
    }

    void respond_the_read_message(std::string message)
    {
        std::regex backtest_pattern("^^([0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}) to ([0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2})");
        std::smatch sm;
        if (std::regex_match(message, sm, backtest_pattern))
        {
            std::string start_date = sm[1];
            std::string end_date = sm[2];
            std::cout << "Parsed data for backtesting:" << start_date << " " << end_date << std::endl;
            // conduct backtest
            /*
            Add backtest code below!
            */
            string type = "1";
            string result = backtest(&start_date, &end_date, type);
            //myTradingInterface.portfolio;

            // send result
            //std::string backtest_result = "backtest:{}";
            auto backtest_result = std::make_shared<beast::flat_buffer>();
            boost::beast::ostream(*backtest_result) << result;
            ws_.write(backtest_result->data());
            //ws_.async_write(backtest_result->data(), [](beast::error_code ec, std::size_t bytes_transferred) {});
        }
    }

    void do_write()
    {
        //return;
        /*
        Add live trading codes
        */
        string result;
        if (time_keeper % 2 == 0) {
            SYSTEMTIME sys;
            GetLocalTime(&sys);
            char yearStr[4], monthStr[2], dayStr[2], hourStr[2], minStr[2], secondStr[2];
            sprintf(yearStr, "%04d", sys.wYear);
            sprintf(monthStr, "%02d", sys.wMonth);
            sprintf(dayStr, "%02d", sys.wDay);
            sprintf(hourStr, "%02d", sys.wHour);
            sprintf(minStr, "%02d", sys.wMinute);
            sprintf(secondStr, "%02d", sys.wSecond);
            string end_year(yearStr), end_month(monthStr), end_day(dayStr), end_hour(hourStr), end_minute(minStr), end_second(secondStr);
            string space = " ";
            string colon = ":";
            string bar = "-";
            string live_end_date = end_year + bar + end_month + bar + end_day + space + end_hour + colon + end_minute + colon + end_second;
            string type;
            if (time_keeper == 0)
                type = "0";
            else
                type = "2";
            //last_end = get_epoch_time(live_end_date);
            result = livetrade(&live_start_date, &live_end_date, type);            
            live_start_date = live_end_date;
            time_keeper = 1;
        }
        else {
            result = "{\"type\":-1}";
        }
        auto live_data = std::make_shared<beast::flat_buffer>();
        boost::beast::ostream(*live_data) << result;
        
        //ws_.write(live_data->data());
        //do_read();
        
        ws_.async_write(live_data->data(), [this](beast::error_code ec, std::size_t bytes_transferred) {
            for (int i = 0; i <= 1000000000; i++)
            {
                i = i; 
                continue;
            }
            time_keeper++;          
            do_read();
            });
        
        
    }

    void on_write(beast::error_code ec, std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if (ec)
            return fail(ec, "write");

        // Clear the buffer
        buffer_.consume(buffer_.size());
        std::cout << "reach on_write!\n";
        std::cout << beast::buffers_to_string(buffer_.data());
        // Do another read
        do_read();
    }
};

//------------------------------------------------------------------------------

// Handles an HTTP server connection
class http_session : public std::enable_shared_from_this<http_session>
{
    // This queue is used for HTTP pipelining.
    class queue
    {
        enum
        {
            // Maximum number of responses we will queue
            limit = 8
        };

        // The type-erased, saved work item
        struct work
        {
            virtual ~work() = default;
            virtual void operator()() = 0;
        };

        http_session& self_;
        std::vector<std::unique_ptr<work>> items_;

    public:
        explicit queue(http_session& self) : self_(self)
        {
            static_assert(limit > 0, "queue limit must be positive");
            items_.reserve(limit);
        }

        // Returns `true` if we have reached the queue limit
        bool is_full() const
        {
            return items_.size() >= limit;
        }

        // Called when a message finishes sending
        // Returns `true` if the caller should initiate a read
        bool on_write()
        {
            BOOST_ASSERT(!items_.empty());
            auto const was_full = is_full();
            items_.erase(items_.begin());
            if (!items_.empty())
                (*items_.front())();
            return was_full;
        }

        // Called by the HTTP handler to send a response.
        template<bool isRequest, class Body, class Fields>
        void operator()(http::message<isRequest, Body, Fields>&& msg)
        {
            // This holds a work item
            struct work_impl : work
            {
                http_session& self_;
                http::message<isRequest, Body, Fields> msg_;

                work_impl(http_session& self,
                    http::message<isRequest, Body, Fields>&& msg) : self_(self), msg_(std::move(msg)) {}

                void operator()()
                {
                    http::async_write(
                        self_.stream_,
                        msg_,
                        beast::bind_front_handler(
                            &http_session::on_write,
                            self_.shared_from_this(),
                            msg_.need_eof()));
                }
            };

            // Allocate and store the work
            items_.push_back(
                boost::make_unique<work_impl>(self_, std::move(msg)));

            // If there was no previous work, start this one
            if (items_.size() == 1)
                (*items_.front())();
        }
    };

    beast::tcp_stream stream_;
    beast::flat_buffer buffer_;
    std::shared_ptr<std::string const> doc_root_;
    queue queue_;

    // The parser is stored in an optional container so we can
    // construct it from scratch it at the beginning of each new message.
    boost::optional<http::request_parser<http::string_body>> parser_;

public:
    // Take ownership of the socket
    http_session(
        tcp::socket&& socket,
        std::shared_ptr<std::string const> const& doc_root)
        : stream_(std::move(socket))
        , doc_root_(doc_root)
        , queue_(*this)
    {
    }

    // Start the session
    void run()
    {
        // We need to be executing within a strand to perform async operations
        // on the I/O objects in this session. Although not strictly necessary
        // for single-threaded contexts, this example code is written to be
        // thread-safe by default.
        net::dispatch(
            stream_.get_executor(),
            beast::bind_front_handler(
                &http_session::do_read,
                this->shared_from_this()));
    }


private:
    void do_read()
    {
        // Construct a new parser for each message
        parser_.emplace();

        // Apply a reasonable limit to the allowed size
        // of the body in bytes to prevent abuse.
        parser_->body_limit(10000);

        // Set the timeout.
        stream_.expires_after(std::chrono::seconds(100000000));

        // Read a request using the parser-oriented interface
        http::async_read(
            stream_,
            buffer_,
            *parser_,
            beast::bind_front_handler(
                &http_session::on_read,
                shared_from_this()));
    }

    void on_read(beast::error_code ec, std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        // This means they closed the connection
        if (ec == http::error::end_of_stream)
            return do_close();

        if (ec)
            return fail(ec, "read");

        // See if it is a WebSocket Upgrade
        if (websocket::is_upgrade(parser_->get()))
        {
            // Create a websocket session, transferring ownership
            // of both the socket and the HTTP request.
            std::make_shared<websocket_session>(
                stream_.release_socket())->do_accept(parser_->release());
            return;
        }

        // Send the response
        handle_request(*doc_root_, parser_->release(), queue_);

        // If we aren't at the queue limit, try to pipeline another request
        if (!queue_.is_full())
            do_read();
    }

    void on_write(bool close, beast::error_code ec, std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if (ec)
            return fail(ec, "write");

        if (close)
        {
            // This means we should close the connection, usually because
            // the response indicated the "Connection: close" semantic.
            return do_close();
        }

        // Inform the queue that a write completed
        if (queue_.on_write())
        {
            // Read another request
            do_read();
        }
    }

    void do_close()
    {
        // Send a TCP shutdown
        beast::error_code ec;
        stream_.socket().shutdown(tcp::socket::shutdown_send, ec);

        // At this point the connection is closed gracefully
    }
};

//------------------------------------------------------------------------------

// Accepts incoming connections and launches the sessions
class listener : public std::enable_shared_from_this<listener>
{
    net::io_context& ioc_;
    tcp::acceptor acceptor_;
    std::shared_ptr<std::string const> doc_root_;

public:
    listener(
        net::io_context& ioc,
        tcp::endpoint endpoint,
        std::shared_ptr<std::string const> const& doc_root)
        : ioc_(ioc), acceptor_(ioc), doc_root_(doc_root)
    {
        beast::error_code ec;

        // Open the acceptor
        acceptor_.open(endpoint.protocol(), ec);
        if (ec)
        {
            fail(ec, "open");
            return;
        }

        // Allow address reuse
        acceptor_.set_option(net::socket_base::reuse_address(true), ec);
        if (ec)
        {
            fail(ec, "set_option");
            return;
        }

        // Bind to the server address
        acceptor_.bind(endpoint, ec);
        if (ec)
        {
            fail(ec, "bind");
            return;
        }

        // Start listening for connections
        acceptor_.listen(net::socket_base::max_listen_connections, ec);
        if (ec)
        {
            fail(ec, "listen");
            return;
        }
    }

    // Start accepting incoming connections
    void run()
    {
        // We need to be executing within a strand to perform async operations
        // on the I/O objects in this session. Although not strictly necessary
        // for single-threaded contexts, this example code is written to be
        // thread-safe by default.
        net::dispatch(
            acceptor_.get_executor(),
            beast::bind_front_handler(
                &listener::do_accept,
                this->shared_from_this()));
    }

private:
    void do_accept()
    {
        // The new connection gets its own strand
        acceptor_.async_accept(
            net::make_strand(ioc_),
            beast::bind_front_handler(
                &listener::on_accept,
                shared_from_this()));
    }

    void on_accept(beast::error_code ec, tcp::socket socket)
    {
        if (ec)
        {
            fail(ec, "accept");
        }
        else
        {
            // Create the http session and run it
            std::make_shared<http_session>(
                std::move(socket),
                doc_root_)->run();
        }

        // Accept another connection
        do_accept();
    }
};

//------------------------Added by Kuant 2021/4/4------------------------------------------------------
void sync_wait_handle(const boost::system::error_code& error_code, boost::asio::deadline_timer* timer)
{
    timer->expires_at(timer->expires_at() + boost::posix_time::seconds(5));
    timer->async_wait(boost::bind(sync_wait_handle, boost::asio::placeholders::error, timer));
}



int main(int argc, char* argv[])
{
    // Check command line arguments.
    /*
    if (argc != 5)
    {
        std::cerr <<
            "Usage: advanced-server <address> <port> <doc_root> <threads>\n" <<
            "Example:\n" <<
            "    advanced-server 0.0.0.0 8080 . 1\n";
        return EXIT_FAILURE;
    }
    */
    auto const address = net::ip::make_address("127.0.0.1");
    auto const port = static_cast<unsigned short>(12345);
    auto const doc_root = std::make_shared<std::string>(".");
    int const threads = 4;

    // The io_context is required for all I/O
    net::io_context ioc{ threads };

    // Create and launch a listening port
    std::make_shared<listener>(
        ioc,
        tcp::endpoint{ address, port },
        doc_root)->run();

    // Capture SIGINT and SIGTERM to perform a clean shutdown
    net::signal_set signals(ioc, SIGINT, SIGTERM);
    signals.async_wait(
        [&](beast::error_code const&, int)
        {
            // Stop the `io_context`. This will cause `run()`
            // to return immediately, eventually destroying the
            // `io_context` and all of the sockets in it.
            ioc.stop();
        });

    // Run the I/O service on the requested number of threads
    std::vector<std::thread> v;
    v.reserve(threads - 1);
    for (auto i = threads - 1; i > 0; --i)
        v.emplace_back(
            [&ioc]
            {
                ioc.run();
            });
    ioc.run();

    // (If we get here, it means we got a SIGINT or SIGTERM)

    // Block until all the threads exit
    for (auto& t : v)
        t.join();

    return EXIT_SUCCESS;
}

/*
int main()
{
	vector<string> symbol_list{ "BTC" };
	vector<string> benchmarksymbols{ "BTC" };
	double initial_cap(1000000);
	string start_date("2021-05-03 10:00:00");
	string end_date("2021-05-03 21:00:00");

	vector<string>* symbol_list_ptr = &symbol_list;
	double* initial_cap_ptr = &initial_cap;
	string* start_date_ptr = &start_date;
	string* end_date_ptr = &end_date;
	long _last_period = 10;
	TradingInterface myTradingInterface(symbol_list_ptr, benchmarksymbols, initial_cap_ptr, start_date_ptr, end_date_ptr, _last_period);

	MainStrategy strategy(&myTradingInterface.pipeline, &myTradingInterface.events);
	Benchmark i_benchmark(&myTradingInterface.benchmarkpipeline, &myTradingInterface.events);

	myTradingInterface.runbacktest(strategy, i_benchmark);

}
*/

