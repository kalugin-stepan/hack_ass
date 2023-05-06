import socket
import cv2
import numpy as np
from threading import Thread

client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client.connect(('192.168.0.105', 5000))
client.send(b'\x02')

is_running = True
DATA = None

def show_stream():
    while is_running:
        if DATA == None: continue
        img = cv2.imdecode(np.frombuffer(DATA, np.uint8), cv2.IMREAD_COLOR)
        try:
            cv2.imshow('lansky gay', img)
        except:
            continue
        cv2.waitKey(5)

Thread(target=show_stream).start()

data = None

while True:
    try:
        cur_data = client.recv(1024)
        if data == None:
            start = cur_data.find(b'\xff\xd8')
            if start != -1:
                data = cur_data[start:]
            continue
        end = cur_data.find(b'\xff\xd9')
        if end != -1:
            data += cur_data[:end+2]
            DATA = data
            cur_data = cur_data[end+3:]
            start = cur_data.find(b'\xff\xd9')
            if start != -1:
                data = cur_data[start:]
            else:
                data = None
            continue
        data += cur_data
    except KeyboardInterrupt:
        break
    except Exception as e:
        print(e)
        break

is_running = False