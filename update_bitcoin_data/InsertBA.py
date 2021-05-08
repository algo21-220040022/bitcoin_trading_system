
import ccxt
import pandas as pd
import time
import pymysql

exchange = ccxt.aax()
symbol = 'BTC/USDT'

conn = pymysql.connect(host="127.0.0.1", user="root",password="123456",database="btc" )
cur = conn.cursor()


delay = 2 # seconds
limit = 50
while True:
    orderbook = exchange.fetch_order_book(symbol,limit)
    bids = orderbook['bids']
    asks = orderbook['asks']
    #timestamp = orderbook['timestamp']
    datetime = orderbook['datetime']
    
    for i in range(len(bids)):
        price = bids[i][0]
        quantity = bids[i][1]
        cur.execute("INSERT INTO bids VALUES(%s,%s,%s)",(tstamp,price,quantity))

    for j in range(len(asks)):
        price = asks[i][0]
        quantity = asks[i][1]
        cur.execute("INSERT INTO asks VALUES(%s,%s,%s)",(datetime,price,quantity))
    
    time.sleep (delay) # rate limit
    conn.commit()
cur.close()
conn.close()


# In[30]:


conn.commit()
cur.close()
conn.close()


# In[34]:

# test
'''
conn = psycopg2.connect(database = 'Market_Info',user = 'postgres',password = '261310carrot~',
                       host = 'localhost',port = '5432')
cur = conn.cursor()
cur.execute("SELECT * FROM Bids")
result = cur.fetchall()
print(result)
conn.commit()
cur.close()
conn.close()
'''


# In[ ]:




