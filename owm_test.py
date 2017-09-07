import datetime
import requests
import time
import sqlite3
from geopy import Nominatim

import store_it


def post_value():
    url = 'http://api.openweathermap.org/data/3.0/measurements?appid=e229425a7ad4dc9b361f341f7ce03904'
    payload = [{
    "station_id": "58da60d7be85b200010eee82",
    "dt": int(time.time()),
    "temperature": 1.7,
    "wind_speed": 1.9,
    "wind_deg":28,
    "humidity": 80,
    }]


    # GET with params in URL
    r = requests.post(url, json=payload)
    print(r.status_code)
    print(r.text)


def get_value():
    url = 'http://api.openweathermap.org/data/3.0/measurements?appid=e229425a7ad4dc9b361f341f7ce03904'

    date_entry = input('Enter a date in YYYY-MM-DD-HH-MM format')
    year, month, day, hours, minutes = map(int, date_entry.split('-'))
    from_date0 = datetime.datetime(year, month, day, hours, minutes)

    #date_entry1 = input('Enter a date in YYYY-MM-DD-HH-MM format')
    #year1, month1, day1, hours1, minutes1 = map(int, date_entry1.split('-'))
    #to_date1 = datetime.datetime(year1, month1, day1, hours1, minutes1)

    payload = {
        "type": "m",
        "limit":100,
        "from": int(time.mktime(from_date0.timetuple())),
        "to": int(time.time()) ,
        "station_id": "58da60d7be85b200010eee82",
    }
    # GET with params in URL
    r = requests.get(url,params=payload)
    print(r.status_code, r.text)


def get_stations():
    url = 'http://api.openweathermap.org/data/3.0/stations?appid=e229425a7ad4dc9b361f341f7ce03904'
    # GET with params in URL
    r = requests.get(url)
    print(r.status_code, r.text)


def delete_stations():
    url = 'http://api.openweathermap.org/data/3.0/stations/##################?appid=e229425a7ad4dc9b361f341f7ce03904' #insert your idStation!
    # GET with params in URL
    r = requests.delete(url)
    print(r.status_code, r.text)


def create_station():
    url = 'http://api.openweathermap.org/data/3.0/stations?appid=e229425a7ad4dc9b361f341f7ce03904'

    payload = {
        "external_id": "P_test0",
        "name": "PalermoTestStation",
        "latitude": 38.1156879,
        "longitude": 13.3612,
        "altitude": 20
    }

    #headers = {'Content-Type': 'application/json'}

    # GET with params in URL
    r = requests.post(url, json=payload)
    print(r.status_code,r.text)


def get_value_from_rectangle():
    url = 'http://api.openweathermap.org/data/2.5/box/city?appid=e229425a7ad4dc9b361f341f7ce03904'

    geolocator = Nominatim()
    location = geolocator.geocode("Sicily") #i get the center of city/region espress to lat/lon
    bbox=str(location.longitude-1.8)+','+str(location.latitude-1.7)+','+str(location.longitude+1.6)+','+str(location.latitude+1.2)+','+str(10)
    payload = {
        "bbox": bbox #+-1 on coordinates, it's will move me for 111km in both directions
    }
    # GET with params in URL
    r = requests.get(url, params=payload)
    c=r.json()

    lon=[]
    lat=[]
    t=[]
    id=[]
    temp_max=[]
    pressure=[]
    temp=[]
    humidity=[]
    temp_min=[]
    name=[]
    wind_deg=[]
    wind_speed=[]

    for i in range(0,len(c["list"])-1): #-1?
        lon.append(c["list"][i]["coord"]["Lon"])
        lat.append(c["list"][i]["coord"]["Lat"])
        id.append(c["list"][i]["id"])
        temp_max.append(c["list"][i]["main"]["temp_max"])
        pressure.append(c["list"][i]["main"]["pressure"])
        temp.append(c["list"][i]["main"]["temp"])
        humidity.append(c["list"][i]["main"]["humidity"])
        temp_min.append(c["list"][i]["main"]["temp_min"])
        name.append(c["list"][i]["name"])
        wind_deg.append(c["list"][i]["wind"]["deg"])
        wind_speed.append(c["list"][i]["wind"]["speed"])
        x_t=c["list"][i]["dt"]
        t.append((datetime.datetime.fromtimestamp(x_t)))
        #year.append(t.year)
        #months=t.month
        #day=t.day
        #hour=t.hour
        #minutes=t.minute
        #seconds=t.second
    return name, lon, lat, pressure, temp, humidity, wind_speed, t,id, wind_deg



def get_value_from_userStations():
    lon = []
    lat = []
    t = []
    id = []
    pressure = []
    temp = []
    humidity = []
    name = []
    wind_deg = []
    wind_speed = []
    latest_list = []
    conn = sqlite3.connect('/home/nataraja/Scrivania/OpenData/workspacePY/mysite/mysite/db.sqlite3')
    c = conn.cursor()
    sql = "SELECT username from auth_user"
    for row in c.execute(sql):
        latest_list.append(row)
    context = {'users': latest_list}
    conn.close()

    conn = sqlite3.connect('/home/nataraja/Scrivania/db_weather.sqlite')
    c = conn.cursor()
    wea=[]
    city=[]
    station=dict()
    for i in context['users']:
        sql = "SELECT weather_id,city FROM %s; "%i
        c.execute(sql)
        station['%s'%i] = c.fetchall()

    for key,value in station.items():
        for i in value:
            user_table=(key+"_"+i[0]+"_"+i[1])
            sql = "SELECT * FROM %s WHERE %s.rowid = (SELECT MAX(rowid) FROM %s); " % (user_table,user_table,user_table)
            c.execute(sql)
            nam,te,hum,w_s,dt,pr,deg  = c.fetchone() or (None,0.0,0.0,0.0,None,0.0,0.0)
            print(nam,te,hum,w_s,dt,pr,deg)
            name.append(nam)
            temp.append(te)
            humidity.append(hum)
            wind_speed.append(w_s)
            t.append(dt)
            pressure.append(pr)
            wind_deg.append(deg)

            sql1 = "SELECT lat, lon FROM City WHERE name = '%s';" % user_table
            c.execute(sql1)
            lt, ln = c.fetchone()
            lat.append(float(lt))
            lon.append(float(ln))

    return name, lon, lat, pressure, temp, humidity, wind_speed, t, wind_deg


#loop to get data
def loop():

    #create_station()
    #delete_stations()
    #post_value()
    #onOpenStreetMap.visual(name, lon, lat, pressure, temp, humidity, wind_speed, t)
    #get_stations()
    #name, lon, lat, pressure, temp, humidity, wind_speed, t, id, wind_deg = get_value_from_rectangle()

    #store_it.drop_table(name)
    #store_it.history_table(name)


    while(True):
        try:
            name, lon, lat,pressure, temp, humidity,wind_speed,t,id,wind_deg=get_value_from_rectangle()


    #map to sqlite
            store_it.insert_city(name, lon, lat, id) #if exist IGNORE
            store_it.insert_history_city(name,temp, humidity, wind_speed, t, id,pressure,wind_deg)#if exist IGNORE
            time.sleep(900) #I get data for each 15 minutes and save them
        except:
            time.sleep(60)
            pass

