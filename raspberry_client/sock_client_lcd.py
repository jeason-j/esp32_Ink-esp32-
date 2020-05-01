import socket  
import time  
import sys


def sock_send(ip, port, cmd):  
    try: 
        print ip,port,cmd
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)  
        #sock = socket.socket(socket.SOL_SOCKET, socket.SO_REUSEADDR)
        sock.connect((ip,port))
        #command_last=0  
        sock.send(cmd)
        time.sleep(0.2)
        #sock.send(cmd)  
        print('send ok')
        #time.sleep(1)     
        sock.close()
        return "succ"
    except:
        return "fail"

reload(sys)
sys.setdefaultencoding('utf8')   

ip="127.0.0.1"
port=10090
cmd='test'
if (len(sys.argv)>1):
    cmd=sys.argv[1]
'''
if (len(sys.argv)>1):
    ip=sys.argv[1]
if (len(sys.argv)>2):
    port=int(sys.argv[2])
if (len(sys.argv)>3):
    cmd=sys.argv[3]
'''    
print(sock_send(ip,port,cmd))

