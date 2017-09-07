import paho.mqtt.client as paho
import sqlite3
import datetime

package=0

def on_subscribe(client, userdata, mid, granted_qos):
    print("Subscribed: "+str(mid)+" "+str(granted_qos))

def on_message(client,userdata,msg):
    print("on_message")
    pass

def store_data(client, userdata, msg):
    global package
    package=package+1
    print("Received package",package)
    name_board=(msg.topic).split('/', 1)[0] #I get the name of board
    temperature,humidity=(msg.payload).split()
    #print(name_board, int(temperature),int(humidity))
    #date=(datetime.datetime.now()).strftime("%Y-%m-%d %H:%M:%S")
    date=(datetime.datetime.now())

    '''
    conn = sqlite3.connect('/home/nataraja/Scrivania/Issia&tesi/sweet_home.sqlite')
    c = conn.cursor()
    c.execute("INSERT or IGNORE INTO '%s' VALUES (%d,%d,'%s')" % (name_board,int(temperature),int(humidity),date) )

    conn.commit()
    conn.close()
    '''




client = paho.Client()
#client.on_subscribe = on_subscribe
#client.on_message = on_message
client.username_pw_set("pulsar", "conoscenza")
client.message_callback_add("+/dht11", store_data)
client.connect("10.42.0.1", 8883)
client.subscribe("+/dht11", qos=1)
client.loop_forever()