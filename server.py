import socket
from threading import Thread, Lock

global data, lock
data = None
lock = Lock()
lock.acquire()


def handle_sender(client: socket.socket):
    global data
    client.settimeout(10)
    try:
        while True:
            cur_data = client.recv(1024)
            if data == None:
                start = cur_data.find(b'\xff\xd8')
                if start != -1:
                    data = cur_data[start:]
                continue
            end = cur_data.find(b'\xff\xd9')
            if end != -1:
                data += cur_data[:end+2]
                cur_data = cur_data[end+3:]
                start = cur_data.find(b'\xff\xd9')
                if lock.locked(): lock.release()
                while data != None:
                    pass
                if start != -1:
                    data = cur_data[start:]
                continue
            data += cur_data
    except:
        pass
    data = None

def handle_reciver(client: socket.socket):
    global data, lock
    while True:
        if lock.locked():
            continue
        lock.acquire()
        try:
            for i in range(0, len(data), 1024):
                cur_package_size = 1024 if len(data) - i >= 1024 else len(data) % 1024
                client.send(data[i:i+cur_package_size])
            data = None
        except:
            break
    data = None

server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.bind(('', 5000))
server.listen()

while True:
    client, addr = server.accept()
    client.settimeout(5)
    on_connect_message = client.recv(1)
    if on_connect_message == b'\x01':
        Thread(target=handle_sender, args=(client, )).start()
        continue
    if on_connect_message == b'\x02':
        Thread(target=handle_reciver, args=(client, )).start()