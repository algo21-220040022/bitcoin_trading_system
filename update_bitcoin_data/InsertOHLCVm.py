#!/usr/bin/env python
# coding: utf-8

# In[1]:


import ccxt
import pandas as pd
import time
import psycopg2


# In[9]:


exchange = ccxt.aax()
symbol = 'BTC/USDT'


# In[10]:


conn = psycopg2.connect(database = 'Market_Info',user = 'postgres',password = '261310carrot~',
                       host = 'localhost',port = '5432')
cur = conn.cursor()


# In[11]:


delay = 60 # seconds
while True:
    candle_list = exchange.fetch_ohlcv (symbol, '1m',limit = 1)  # minute
    data = candle_list[0]
    timestamp = data[0]
    o = data[1]
    h = data[2]
    l = data[3]
    c = data[4]
    volume = data[5]
    cur.execute("INSERT INTO CandlestickMinute VALUES(%s,%s,%s,%s,%s,%s)",(timestamp,o,h,l,c,volume))    
    conn.commit()
    time.sleep (delay) # rate limit


# In[13]:


conn.commit()
cur.close()
conn.close()


# In[ ]:




