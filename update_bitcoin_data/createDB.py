#!/usr/bin/env python
# coding: utf-8

# In[1]:


import psycopg2


# In[5]:


conn = psycopg2.connect(database = 'BTCtest',user = 'postgres',password = '261310carrot~',
                       host = 'localhost',port = '5432')
cur = conn.cursor()

sql1 = "CREATE TABLE TradeOrder        \
(OrderID varchar(225) NOT NULL,        \
 OrderType varchar(10),       \
 Side varchar(10),         \
 Quantity float(50),         \
 Price float(50),         \
 Time varchar(225),        \
 PRIMARY KEY(OrderID))"
cur.execute(sql1)

#CREATE TABLE tradeorder(orderid varchar(225) not null,ordertype varchar(10), totalvalue float(50), quantity float(50), price float(50), time varchar(225), PRIMARY KEY(orderid))

sql2 = "CREATE TABLE CancelOrder        (OrderID varchar(225) NOT NULL,          OrderType varchar(10),         Side varchar(10),         Quantity float(50),          Price float(50),          Time varchar(225),          PRIMARY KEY(OrderID),         FOREIGN KEY (OrderID) REFERENCES TradeOrder(OrderID))"
cur.execute(sql2)

#CREATE TABLE cancelorder(orderid varchar(225) NOT NULL, orderType varchar(10), totalvalue float(50), quantity float(50), price float(50),time varchar(225),PRIMARY KEY(orderid),FOREIGN KEY (orderid) REFERENCES tradeorder(orderid))


sql3 = "CREATE TABLE ExecuteOrder        (OrderID varchar(225) NOT NULL,          OrderType varchar(10),         Side varchar(10),         TransactionCost float(50),         Quantity float(50),          Price float(50),          Time varchar(225),          PRIMARY KEY(OrderID),         FOREIGN KEY (OrderID) REFERENCES TradeOrder(OrderID))"
cur.execute(sql3)

CREATE TABLE executeorder(orderid varchar(225) NOT NULL,ordertype varchar(10),totalvalue float(50),transactioncost float(50),quantity float(50),price float(50),time varchar(225),PRIMARY KEY(orderid),FOREIGN KEY (orderid) REFERENCES tradeorder(orderid))


sql4 = "CREATE TABLE Account        (DailyRuturn float(50) ,          WeeklyReturn float(50),         MonthlyReturn float(50),          YearlyReturn float(50),          TotalReturn float(50),          TotalAsset float(50),         Position float(50))"
cur.execute(sql4)

CREATE TABLE account(DailyRuturn float(50) ,          WeeklyReturn float(50),         MonthlyReturn float(50),          YearlyReturn float(50),          TotalReturn float(50),          TotalAsset float(50),         Position float(50))"

conn.commit()
cur.close()
conn.close()


# In[8]:


conn2 = psycopg2.connect(database = 'Market_Info',user = 'postgres',password = '261310carrot~',
                       host = 'localhost',port = '5432')
cur2 = conn2.cursor()

sql5 = "CREATE TABLE Bids        (TimeStamp varchar(225) ,          bid_price float(50),         bid_quantity float(50))"
cur2.execute(sql5)
CREATE TABLE bids(time varchar(225),bid_price float(50),bid_quantity float(50))

sql6 = "CREATE TABLE Asks        (TimeStamp varchar(225) ,          ask_price float(50),         ask_quantity float(50))"
cur2.execute(sql6)

sql7 = "CREATE TABLE CandlestickDay        (TimeStamp varchar(225) ,          Open float(50),         High float(50),         Low float(50),         Close float(50),         Volume float(50))"
cur2.execute(sql7)

sql8 = "CREATE TABLE CandlestickHour        (TimeStamp varchar(225) ,          Open float(50),         High float(50),         Low float(50),         Close float(50),         Volume float(50))"
cur2.execute(sql8)

sql8 = "CREATE TABLE CandlestickMinute        (TimeStamp varchar(225) ,          Open float(50),         High float(50),         Low float(50),         Close float(50),         Volume float(50))"
cur2.execute(sql8)

conn2.commit()
cur2.close()
conn2.close()


# In[ ]:





# In[ ]:




