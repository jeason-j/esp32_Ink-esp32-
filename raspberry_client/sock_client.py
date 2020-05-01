#!/usr/bin/env python  
import socket  
import time  
import sys

def sock_send(ip, port, cmd):  
    try: 
        #reload(sys)
        #sys.setdefaultencoding('utf8')
        print( "begin:", ip, port,  cmd)
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)  
        print( "connect:")
        sock.connect((ip,port))
        #command_last=0  
        print ("send:")
        sock.send(cmd)  
        #print 'cmd send ok'
        time.sleep(0.2)     
        sock.close()
        print ("sock_send succ")
        return "succ"
    except BaseException as Argument:
        print ("sock_send fail",Argument)
        return "fail:"+ str(Argument)
  
def sock_send2(port, cmd):  
    try: 
        #reload(sys)
        #sys.setdefaultencoding('utf8')
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)  
        sock.connect(("127.0.0.1",port))
        #command_last=0  
        sock.send(cmd)  
        #print 'cmd send ok'
        time.sleep(1)     
        sock.close()
        return "succ"
    except:
        return "fail:"+str(Argument)
   


#port=10004
#cmd='test'
#if (len(sys.argv)>1):
#    port=int(sys.argv[1])
#if (len(sys.argv)>2):
#    cmd=sys.argv[2]
#sock_send(port,cmd)

