import datetime

import time

from sys import exit

import paho.mqtt.client as paho

import struct

import sqlite3


global board_list

board_list=[]

def init_db():

    #omissis

    pass



def add_to_db(buffer_b):

    #omissis

    pass



def on_subscribe(client, userdata, mid, granted_qos):

    print("Subscribed: " + str(mid) + " " + str(granted_qos))


def hello(client, userdata, msg):

    board_name=(msg.topic).split('/', 1)[0]

    board_list.append(msg.topic)

    print("Hello client, this is ", board_name, " Answer: ", msg.payload)



def store_data(client, userdata, msg):
    buffer_b=bytearray()

    buffer_b.extend(msg.payload)

    #store new data

    add_to_db(buffer_b)

    #print new formatted data

    board_name=(msg.topic).split('/', 1)[0]

    board_id=ord(msg.payload[1])

    board_type=ord(msg.payload[2])
    date = '{:02d} {:02d} {:02d} {:02d} {:02d} {:02d}'.format(*[bcdToInt(i) for i in msg.payload[3:9]])
    #date=msg.payload[3:9]

    date = datetime.datetime.strptime(date,"%y %m %d %H %M %S")


    #buffer_b = bytearray(msg.payload[9:53])
    (Vdc, Vrms, Idc, Irms, Pdc, P, A, T, wire, i2c, spi) = struct.unpack_from("!fffffffffff", buffer_b, 9)

    fmtString = "{:.2f}\t" * 11
    print("{} id: {} type: 0x{:0x} -- {}".format(board_name, board_id, board_type, date))
    print(fmtString.format(Vdc, Vrms, Idc, Irms, Pdc, P, A, T, wire, i2c, spi))



def bcdToInt(bcdStr):
    bcd, = struct.unpack('b', bcdStr)
    bcdInt = (bcd >> 4) * 10 + (bcd & 0x0f)
    return bcdInt

#main code

print("Store_it module active")

print("discovering boards...")

client = paho.Client()
client.username_pw_set("issia", "cnr")
client.connect("150.145.127.45", 8883)





client.subscribe("wemos0/hello",qos=0)

client.message_callback_add("wemos0/hello", hello)



#send a broadcast request on topic /requestHello

x=struct.pack("ccc",chr(0x68),chr(0xFF),chr(0x00))

client.publish("/requestHello", x, 0)



#check for answers

client.loop_start()

start=time.time()

#allow 10 seconds for all wemos to respond

while (time.time() - start < 10.0):

   # print("waiting...")

    pass



client.loop_stop()

client.disconnect()



board_number = len(board_list)

if (board_number == 0):

    print("No board answered")

    exit()

else:

    print(str(board_number) + " boards answered")



#initialize database

init_db();



print("running...")



#initialized and starting new client for multiple subscriptions

client = paho.Client()
client.username_pw_set("issia", "cnr")
client.connect("150.145.127.45", 8883)
client.subscribe("+/data", qos=0)
client.subscribe("+/hello", qos=0)

client.message_callback_add("+/data", store_data)
client.message_callback_add("+/hello", welcome)
client.loop_forever()
