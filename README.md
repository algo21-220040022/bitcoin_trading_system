# bitcoin_trading_system
This bitcoin trading system includes Tencent cloud, database, backtest, live trading and a front page to show the result.

## Content
* Preparation
* System Frame
* Database
* Back-End
* Front-End

## Preparation
In order not the occupy the local memory and record the ticker data, this project uses Linux system on a Tencent Cloud to continuously download and store the data. 

## System Frame
THe database is allocated on cloud and use the code in update_data file to download ticker data to the database. The back-end and front-end are allocated on local computer. The back-end is written in C++ and conduct two main functions--backtest and live trading. When it runs the backtest and live trading, it would query data from database on cloud. It is also connected to front-end throught websocket. The front-end is a html file written in html/css/javascript. The client can input some parameters of backtest in the front page and send them to the back-end, which would run the backtest and send backtest results in reply. The back-end would also continuously run the live trading and send the acount information to the front page.

## Database
We construct two databases in MySQL. One is used to store the user information and another to store historical and live ticker data and order book data.

## Back-end
In this part, we develop a event-driven frame to run the backtest and live trading. Besides, in order to communicate with MySQL in cloud, we also use "mysql.h" to build the connection. And for communication with front-end, we use websocket to do the job.

## Front-end
In this part, we divide the front page into two parts. One part is to input the backtest parameter and show the backtest results. Another is to show the live trading results.
