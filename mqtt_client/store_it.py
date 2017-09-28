import paho.mqtt.client as paho
import sqlite3
import datetime


def on_subscribe(client, userdata, mid, granted_qos):
    print("Subscribed: "+str(mid)+" "+str(granted_qos))



def welcome(client, userdata, msg):
    #name_board=(msg.topic).split('/', 1)[0]
    board_list=[]
    board_list.append(msg.payload)
    print 'Hello client, i\'m ',msg.payload

def store_data(client, userdata, msg):
    #print((msg.payload))
    buffer_b=bytearray()
    buffer_b.extend(msg.payload)
    import struct
    x=struct.unpack_from('cccc',buffer_b,9)
    print [ord(i) for i in x]
    x=struct.unpack_from('!f',buffer_b,9)
    print x
    #global package
    #package=package+1
    '''
    name_board=(msg.topic).split('/', 1)[0] #I get the name of board
    id_board=(msg.payload.split(' ')[1])
    board_type=(msg.payload.split(' ')[2])
    date=(msg.payload.split(' ')[3:9])
    date=' '.join(date)
    #Vmeans,Vrms,Imeans,Irms,A,P,T,f,wire,i2c,spi=\
    Vmean3,Vmean2,Vmean1,Vmeans0=msg.payload.split(' ')[9:13]
    elements=[Vmean3,Vmean2,Vmean1,Vmeans0]
    buffer_b=bytearray(elements)
    import struct
    x=struct.unpack_from('f',buffer_b)
    print x
    '''



    #print(name_board, datetime.datetime.strptime(date,"%y %m %d %H %M %S"),
          #Vmeans,Vrms,Imeans,Irms,A,P,T,f,wire,i2c,spi)

    #date=(datetime.datetime.now())

    '''
    conn = sqlite3.connect('/home/nataraja/Scrivania/Issia&tesi/sweet_home.sqlite')
    c = conn.cursor()
    c.execute("INSERT or IGNORE INTO '%s' VALUES (%d,%d,'%s')" % (name_board,int(temperature),int(humidity),date) )

    conn.commit()
    conn.close()
    '''



print("Storage module active")
client = paho.Client()
client.username_pw_set("issia", "cnr")
client.connect("150.145.127.37", 8883)
client.publish("/requestWelcomeToServer",'h',0)
client.loop_start()
client.loop_stop()
client.disconnect()

    #Initialized and starting new client for multiple subscribe
client = paho.Client()
client.username_pw_set("issia", "cnr")
client.connect("150.145.127.37", 8883)
client.subscribe("+/data", qos=0)
client.subscribe("/welcome", qos=0)
client.message_callback_add("+/data", store_data)
client.message_callback_add("+/welcome", welcome)
client.loop_forever()