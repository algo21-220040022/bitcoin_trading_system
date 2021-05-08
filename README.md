# bitcoin_trading_system
This bitcoin trading system includes Tencent cloud, database, backtest, live trading and a front page to show the result.

## Content
* Preparation
* System Frame
* Database
* Back-End
* Front-End
* Further discussion

## Preparation
In order not the occupy the local memory and record the ticker data, this project uses Linux system on a Tencent Cloud to continuously download and store the data. 

## System Frame
THe database is allocated on cloud and use the code in update_data file to download ticker data to the database. The back-end and front-end are allocated on local computer. The back-end is written in C++ and conduct two main functions--backtest and live trading. When it runs the backtest and live trading, it would query data from database on cloud. It is also connected to front-end throught websocket. The front-end is a html file written in html/css/javascript. The client can input some parameters of backtest in the front page and send them to the back-end, which would run the backtest and send backtest results in reply. The back-end would also continuously run the live trading and send the acount information to the front page.

## Database
For this repo, relational database is able to satisfy our demands so we choose from two open-source free relational database model: mySQL and postgreSQL. We test the time that spent to insert and select data and the result is

  *Table: time to insert data*    

time(s)|1 row|10 rows|100 rows|... 
-------|-----|-------|--------|--- 
mySQL  | 0.278513877 | 2.784136772  | 27.62212780  | ...  
postgreSQL  | 0.272163891  | 2.716496070  | 26.97019375  | ...  

  *Table: time to select data*    

time(s)|10 rows|100 rows|1000 rows|... 
-------|-----|-------|--------|--- 
mySQL  | 0.000277579 | 0.001202503  | 0.008962539  | ...  
postgreSQL  | 0.000115829  | 0.000267737  | 0.001757550  | ...  

We can see that postgreSQL is always faster than mySQL especially when extracting data, almost 10 times faster. Therefore, the better choice for database is postgreSQL. However, in this case, there is some problem when constructing the environment on Linux so we use mySQL for the moment. 

We create two database, one is used to store the user information and another to store historical and live ticker data and order book data. The relational schemas are
*The relational schema of user_info*
![user_info](https://github.com/algo21-116010293/bitcoin_trading_system/blob/main/pictures/User_info.png)
*The relational schema of market_info*
![market_info](https://github.com/algo21-116010293/bitcoin_trading_system/blob/main/pictures/Market_info.png)

## Back-end
In this part, we develop a event-driven frame to run the backtest and live trading. Besides, in order to communicate with MySQL in cloud, we also use "mysql.h" to build the connection. And for communication with front-end, we use websocket to do the job. The complete codes are in the [head_file](https://github.com/algo21-116010293/bitcoin_trading_system/tree/main/head_file) and [source_file](https://github.com/algo21-116010293/bitcoin_trading_system/tree/main/source_file)

## Front-end
In this part, we divide the front page into two parts. One part is to input the backtest parameter and show the backtest results. Another is to show the live trading results.
The result page is
*The front page*
![page](https://github.com/algo21-116010293/bitcoin_trading_system/blob/main/pictures/page.png)
 
 To see the complete demo, see the .mp4 file [demo](https://github.com/algo21-116010293/bitcoin_trading_system/blob/main/demo.mp4)
 
 ## Further discussion
 After discussing, there may be several points we can further improve. 
* **Database: change the mySQL to postgreSQL. Improve the efficiency when extracting data from backend.**  

* **Paper trading. Because of time reason, we have not utilize the event-driven framework to run the paper & live trading process. We can add a monitoring exchange to monitor trading.**
* **System Frame. We can divide the system into research part and trading part because when backtesting, the requirement of speed is not high and we can use python the achieve it and improve the efficiency of writing code. As for trading part, still use C++ since it is much faster**
