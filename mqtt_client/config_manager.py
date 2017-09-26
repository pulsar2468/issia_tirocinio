import re
import subprocess
import paho.mqtt.client as paho
import sys

try:
    reply=raw_input('Do you want programming? y/n')
    flag=False

    if (re.search('[yes|y|Y|Yes|YES]',reply)):

        client = paho.Client()
        client.connect("150.145.127.37", 8883)
        client.publish("+/config", "eeprom_data", 0)
        flag=True #i verify if the data has been send to boards
        pass

    elif (re.search('[no|N|n|No|NO]',reply) or flag):
        print("Starting module store_it")
        proc=subprocess.Popen("/home/nataraja/Scrivania/Issia\&tesi/mqtt_client/store_it.py", shell=True)
        if not proc:
            print("Error to create a new process. Exit")
            sys.exit(0)
        print("Process created Pid:" +str(proc.pid))
        print("config_manager exit")
        #sys.exit(0)
except:
    print("Error into config_manage.py. Please reboot module")