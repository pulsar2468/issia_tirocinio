import paho.mqtt.client as paho
import sqlite3
import datetime


def on_subscribe(client, userdata, mid, granted_qos):
    print("Subscribed: "+str(mid)+" "+str(granted_qos))

def on_message(client,userdata,msg):
    print("on_message")
    pass

def store_data(client, userdata, msg,hostname):
    print(userdata)
    #global package
    #package=package+1
    #print("Received package",package)
    name_board=(msg.topic).split('/', 1)[0] #I get the name of board
    date,Vmeans,Vrms,Imeans,Irms,A,P,T,f=(msg.payload).split()
    #date = date + time
    print(name_board, datetime.datetime.strptime(date.decode("utf-8") ,"%d-%m-%y%H:%M:%S"),
          float(f),float(T),float(Vmeans),float(Vrms))
    #date=(datetime.datetime.now()).strftime("%Y-%m-%d %H:%M:%S")
    #date=(datetime.datetime.now())

    '''
    conn = sqlite3.connect('/home/nataraja/Scrivania/Issia&tesi/sweet_home.sqlite')
    c = conn.cursor()
    c.execute("INSERT or IGNORE INTO '%s' VALUES (%d,%d,'%s')" % (name_board,int(temperature),int(humidity),date) )

    conn.commit()
    conn.close()
    '''


try :
    print("Storage module active")
    client = paho.Client()
    #client.on_subscribe = on_subscribe
    #client.on_message = on_message
    client.username_pw_set("issia", "cnr")
    client.message_callback_add("+/data", store_data)
    client.connect("150.145.127.37", 8883)
    client.subscribe("+/data", qos=0)
    client.loop_forever()
except
    print("Error into Storage module, please restart config_manager")