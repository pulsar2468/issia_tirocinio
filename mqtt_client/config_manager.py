import re
import subprocess

import datetime
import paho.mqtt.client as paho
import sys
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







reply=raw_input('Do you want programming? y/n')
flag=False

if (re.search('[yes|y|Y|Yes|YES]',reply)):

    client = paho.Client()
    client.username_pw_set("issia", "cnr")
    client.connect("150.145.127.37", 8883)
    date=(datetime.datetime.now())
    #datetime.datetime.strftime(date,"%y-%m-%d %H:%M:%S")
    year,month,day,hour,minute,second=decimalToBCD(date)
    print(year,month,day,hour,minute,second)

    x=struct.pack("cccccccc",chr(0x05),chr(0xFF),chr(year),chr(month),chr(day),
                  chr(hour),chr(minute),chr(second)) #data reprogramming
    client.publish("/config" ,x)
    client.loop_start()
    client.loop_stop()
    client.disconnect()
    #flag=True #i verify if the data has been send to boards


elif (re.search('[no|N|n|No|NO]',reply) or flag):
    print("Starting module store_it")
    proc=subprocess.Popen("/home/nataraja/Scrivania/Issia\&tesi/mqtt_client/store_it.py", shell=False)
    if not proc:
        print("Error to create a new process. Exit")
        sys.exit(0)
    print("Process created Pid:" +str(proc.pid))
    print("config_manager exit")
    #sys.exit(0)
