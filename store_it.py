import sqlite3


def history_table(name):
    conn = sqlite3.connect('/home/nataraja/Scrivania/db_weather.sqlite')
    c = conn.cursor()
    sql=""

    for i in name:
        sql = sql + 'ALTER TABLE "%s" ADD "pressure" FLOAT NULL;' \
                    'ALTER TABLE "%s" ADD "wind_deg" FLOAT NULL;' %(i,i)
        #sql = sql +  'CREATE TABLE  "%s" ("name" VARCHAR ,"temp" FLOAT, "humidity" FLOAT, "wind_speed" FLOAT, ' \
        #  '"detection_time" DATETIME PRIMARY KEY, "pressure" FLOAT, "wind_deg" FLOAT);' %i


    #print(sql)
    #sql=sql+'CREATE TABLE  City ("id" VARCHAR PRIMARY KEY, "name" FLOAT, "lat" FLOAT, "lon" FLOAT);'
    c.executescript(sql)
    conn.commit()
    conn.close()

def drop_table(name):
    conn = sqlite3.connect('/home/nataraja/Scrivania/db_weather.sqlite')
    c = conn.cursor()
    sql=""

    for i in name:
        sql = sql +  'DROP TABLE IF EXISTS "%s";' %i

    sql=sql+ 'DROP TABLE IF EXISTS City;'
    conn.executescript(sql)
    conn.close()


def insert_city(name, lon, lat, id):
   conn= sqlite3.connect('/home/nataraja/Scrivania/db_weather.sqlite')
   c = conn.cursor()
   sql = ""

   for i,n,l,ln in zip(id,name,lat,lon):
       sql = sql + 'INSERT or IGNORE INTO City  VALUES ("%s","%s",%f,%f);' %(i,n,l,ln)

   c.executescript(sql)
   conn.commit()
   conn.close()


def insert_history_city(name,temp, humidity, wind_speed, t, id, pressure,wind_deg):
    conn = sqlite3.connect('/home/nataraja/Scrivania/db_weather.sqlite')
    c = conn.cursor()
    sql = ""

    for a1,b1,c1,d1,e1,f1,pr,deg in zip(name,temp, humidity, wind_speed, t,name,pressure,wind_deg):
        sql = sql + 'INSERT or IGNORE INTO "%s" VALUES ("%s",%f,%f,%f,"%s",%f,%f);' % (a1,f1,b1,c1,d1,e1,pr,deg)

    c.executescript(sql)
    conn.commit()
    conn.close()


