import re
import datetime
import time

import paho.mqtt.client as paho
import struct

def decimalToBCD(date):
    u=(date.year % 100) % 10
    d=(date.year % 100) // 10
    c_year=((d << 4) + u)

    u=date.month % 10
    d=date.month // 10
    c_month=((d << 4) + u)

    u=date.day % 10
    d=date.day // 10
    c_day=((d << 4) + u)

    u=date.hour % 10
    d=date.hour // 10
    c_hour=((d << 4) + u)

    u=date.minute % 10
    d=date.minute // 10
    c_minute=((d << 4) + u)

    u=date.second % 10
    d=date.second // 10
    c_second=((d << 4) + u)

    return c_year,c_month,c_day,c_hour,c_minute,c_second


def answer(client, userdata, msg):
    global all_wemos_response
    global package
    package = 0
    name_board=(msg.topic).split('/', 1)[0]
    #board_list=[]
    #board_list.append(msg.payload)
    print("Wemos: ",name_board,"Answer: ", msg.payload)
    package=package+1
    if (package == 1):
        all_wemos_response=True
        package=0




reply=raw_input('Do you want programming? y/n')
flag=False
all_wemos_response=False


if (re.search('[yes|y|Y|Yes|YES]',reply)):
        reply1=raw_input('Datetime or general config? d/c')
        client = paho.Client()
        client.username_pw_set("issia", "cnr")
        client.connect("150.145.127.37", 8883)
        client.subscribe("+/answers",qos=0)
        client.message_callback_add("+/answers", answer)
        if (re.search('[d|D|Datetime|datetime|DATETIME]',reply1)):

            date=(datetime.datetime.now())
            #datetime.datetime.strftime(date,"%y-%m-%d %H:%M:%S")
            year,month,day,hour,minute,second=decimalToBCD(date)
            #print(year,month,day,hour,minute,second)

            x=struct.pack("cccccccc",chr(0x05),chr(0xFF),chr(year),chr(month),chr(day),
                      chr(hour),chr(minute),chr(second)) #data reprogramming
            client.publish("/config" ,x, 0)

        if (re.search('[c|C|Config|CONFIG]',reply1)):
            user="issia"
            passw="cnr"
            ssid="test1"
            wifi_passw="magic"
            y=struct.pack("!I", 8883)
            a,b, port_hi, port_lo = struct.unpack("!cccc", y)
            x=struct.pack("cccccccccc30s30s30s30s",chr(0x63),chr(0xFF),chr(0x21),chr(0xFE),chr(150),
                         chr(145),chr(127),chr(37),port_hi,port_lo,user,passw,ssid,wifi_passw) #data reprogramming

            client.publish("/config" ,x,0)
        client.loop_start()


        #client.loop_start()
        start=time.time()
        #loop until as all wemos response or i receive a timeout, in this way i prevent the wemos damaged
        while not(all_wemos_response) and (time.time() - start < 10.0):
           # print("I'm waiting that all wemos response that all ok")
            pass
        client.loop_stop()
        client.disconnect()
        all_wemos_response=False
        flag=True #i verify if the data has been send to boards


if (re.search('[no|N|n|No|NO]',reply) or flag):
        print("Starting module store_it")
        import store_it # in this case, i call module as subprocess of this process
        '''
        proc=subprocess.Popen("/home/nataraja/Scrivania/Issia\&tesi/mqtt_client/store_it.py", shell=True)
        if not proc:
            print("Error to create a new process. Exit")
        #sys.exit(0)
        print("Process created Pid:" +str(proc.pid))
        print("config_manager exit")
        #sys.exit(0)
        '''