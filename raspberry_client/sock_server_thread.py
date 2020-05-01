#!/usr/bin/env python
# -*- coding: utf-8 -*-
import socket  
import sys  
import threading  
import random  
import os  
import time  
import struct  
import commands   #执行系统命令模块
import threading
import sock_client

#线程方式 sockct服务器， 接收10005端口的数据
#当有数据进入后，会改变 全局变量 last_msg
#引用方只要跟踪 sock_server_thread.last_msg 变量，即可判断是否有数据进入
#退出进程用如下方式
#try
#  ...
#except KeyboardInterrupt:  
#  sock_server_thread.exit_now=1
#  sock_client.sock_send('127.0.0.1',10005,'')

HOST = "0.0.0.0"  
PORT = 10090  
SOCK_ADDR = (HOST, PORT)  
exit_now = 0  
last_msg=""
last_changed=False
reload(sys)
sys.setdefaultencoding('utf8')

class SocketClientObject(object):  
    def __init__(self, socket, address ):  
        self.socket = socket  
        self.address = address  
  
class ClientThread(threading.Thread):  
    def __init__(self, client_object):  
        threading.Thread.__init__(self)  
        self.client_object = client_object  
  
  
    def run(self):  
        global last_msg,last_changed
        self.running = True  
        while self.running:  
            data = self.client_object.socket.recv(1024)  
            #print ">> Received data: ", data, " from: ", self.client_object.address  
            #cmd_status,cmd_result=commands.getstatusoutput(data)   
            #print cmd_status,cmd_result
            #print "data=", data
            if data=="":
                break
            else:
                print ">> Received data: ", data, " from: ", self.client_object.address
                last_msg=data
                last_changed=True
            #net_input(data)           
        #print "client_quit"     
        self.client_object.socket.close()    
  
    def stop(self):  
        self.running = False  
  

def work(param):
    global exit_now
    try:  
        print "thread start"
        #	流式socket , for TCP
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)  
        sock.bind(SOCK_ADDR)  
        #最大连接数量
        sock.listen(1)  
        while exit_now == 0: 
            #print "exit_now",  exit_now  
            # 接受TCP连接并返回 阻塞式 
            (clientsocket, address) = sock.accept()  
            #print " Accept client: ", address  
            ct = ClientThread(SocketClientObject(clientsocket, address))  
            ct.start()  
    except:  
        print "#! EXC: ", sys.exc_info()  
        #sock.close()  
        #ser_th.stop()  
        #ser_th.join()  
        print "THE END! Goodbye!" 
    print "thread end"

def do_thread():  
    sock_t = threading.Thread(target=work,args=(u'',))
    #t.setDaemon(True)
    sock_t.start()
    return sock_t