import socket

HOST = "192.168.9.229"
PORT = 1337

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as client:
    client.connect((HOST, PORT))
    print("Connected to game!")
    while True: 
      data = client.recv(1024)
      if(len(data) <= 3):
          print("Disconnected")
          break
      else:
          print(repr(data)[2:-1] + '\n')
      
            
    
