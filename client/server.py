from http.server import BaseHTTPRequestHandler, HTTPServer
from wakeonlan import send_magic_packet
import socketserver
import mimetypes
import socket
import fcntl
import time
import os

HOST		= "localhost"
serverPORT	= 3653
remotePORTFI    = 3650
remotePORT  = 3653
wakePORT 	= 9
remoteHOSTFI    = ""
remoteHOST 	= ""
remoteMAC 	= ""

with open("IPV4", "r") as f:
  data = f.readlines()
  remoteHOSTFI = data[0][:-1]

with open("IPV4_CA", "r") as f:
  data = f.readlines()
  remoteHOST = data[0][:-1]
  remoteMAC = data[1][:-1]

devices = {
'macMini': {
  'mac': remoteMAC,
  'ip_address': remoteHOST
  }
}

def establishTunnel(service):
  print(f"request for {service}")

  if(service == b'alexandria'):
    print(f"connecting to {remoteHOSTFI}")
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM, 0) as s:
      s.connect((remoteHOSTFI, remotePORTFI))
      s.sendall(service)
      s.settimeout(15)
      data = s.recv(1)
      return (data or False)
    
  print(f"connecting to {remoteHOST}")
  with socket.socket(socket.AF_INET, socket.SOCK_STREAM, 0) as s:
    s.connect((remoteHOST, remotePORT))
    s.sendall(bytes(service, 'ascii'))
    s.settimeout(1)
    data = s.recv(1024)
    return (data or False)

class MyServer(BaseHTTPRequestHandler):
  def do_GET(self):
    if self.path.startswith('/'):
      self.path = 'public' + self.path
    if self.path == 'public/':
      self.path = 'public/index.html'
    try:
      mimetypes.init()
      f = open(self.path, 'rb')
      self.send_response(200)
      mimetype = mimetypes.guess_type(self.path)
      self.send_header("Content-type", mimetype[0])
      self.send_header('Access-Control-Allow-Origin', '*')
      self.end_headers()
      self.wfile.write(f.read())
      f.close();
    except:
      f = "File not found: " + self.path
      self.send_error(404,f)

  def do_POST(self):
    self.wake_devices()
    content_length = int(self.headers['Content-Length'])
    body = self.rfile.read(content_length)
    self.send_response(200)
    self.end_headers()
    flag = establishTunnel(body)
    print( ("sucessfully established " if flag else "failed to establish ") + "tunnel" )
    self.check()

  def wake_devices(self):
    for device, device_info in devices.items():
      mac,ip = device_info.values()
      send_magic_packet(mac,ip_address=remoteHOST,port=wakePORT)
      print('Magic Packet Sent To ' + device)
      print('IPAddress: ' + ip)
      print('MAC Address: ' + mac)
      print('Port: ' + str(wakePORT))

  def check(self):
    try:
      self.log_message("command: %s", self.path)
      if self.path == '/':
        subprocess.run(
          "/usr/bin/ssh -q " + remoteHOST,
          shell=True,
        )
    except BaseException as err:
      print(f"Unexpected {err=}, {type(err)=}")

if __name__ == "__main__":
  webServer = HTTPServer((HOST, serverPORT), MyServer)
  try:
    print("-----------------------------------------------------------")
    print(time.asctime(), "Start Server - %s:%s"%(HOST, serverPORT))
    webServer.serve_forever()
  except KeyboardInterrupt:
    pass
  webServer.server_close()
  print("-----------------------------------------------------------")
  print(time.asctime(),'Stop Server - %s:%s' %(HOST,serverPORT))
