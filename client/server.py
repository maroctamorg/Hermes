from http.server import BaseHTTPRequestHandler, HTTPServer
import socket
import time

localHOST   = "localhost"
serverPORT  = 8080
remotePORT  = 3650

def establishTunnel(service):
    remoteHOST = ""
    with open("IPV4.txt", "r") as f:
        remoteHOST = f.readline()[:-1]
    
    #print(f"connecting to {remoteHOST}")
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((remoteHOST, remotePORT))
        s.sendall(bytes(service, 'ascii'))
        s.settimeout(5) 
        data = s.recv(1)
        return (data or False)

class MyServer(BaseHTTPRequestHandler):
    def do_GET(self):
        self.send_response(200)
        self.send_header("Content-type", "text/html")
        self.end_headers()
        with open('index.html', 'rb') as index:
            self.wfile.write(index.read())

    def do_POST(self):
        content_length = int(self.headers['Content-Length'])
        body = self.rfile.read(content_length)
        #print(body)
        self.send_response(200)
        self.end_headers()
        flag = establishTunnel("s")
        #print( ("sucessfully established" if flag else "failed to establish") + "tunnel" )

if __name__ == "__main__":        
    webServer = HTTPServer((localHOST, serverPORT), MyServer)
    print("Server started http://%s:%s" % (localHOST, serverPORT))
    try:
        webServer.serve_forever()
    except KeyboardInterrupt:
        pass

    webServer.server_close()
    print("Server stopped.")
