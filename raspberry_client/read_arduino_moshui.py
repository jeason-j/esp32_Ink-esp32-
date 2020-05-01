# -*- coding: utf-8 -*
import serial
import time
import os
import re
import sys
import sock_server_thread 
import threading
import sock_client


#用sock方式给墨水屏发命令
time.sleep(2)

# 打开串口
# drrobot 用这个
#ser = serial.Serial("/dev/ttyACM0",921600)
#ttgo板用这个
ser = serial.Serial("/dev/ttyUSB0",921600)
last_curl=time.time()

  
def show_screen(out_str):
    print("show_screen begin")
    #生成图片
    os.system("python3 /home/pi/moshui_bmp.py '" +out_str+"'")
    bmp_size = os.path.getsize('/home/pi/tmp.bmp')
    ser.flushInput() 
    
    #等2秒，作用是唤醒esp32
    print("wake esp32")
    ser.write('wake\r\n')  
    time.sleep(1)        
    
    #1.发送头标志
    ser.write('jpg>\r\n')    
    print("ser.write",'jpg>\r\n')
    last_curl=time.time()
    recv=""
    while True:
      count = ser.inWaiting()
      nowtime=time.time()
      if nowtime-last_curl>2:
        print("show_screen timeout")
        break      
      if count != 0:
        # 读取内容并回显
        recv = ser.read(count)
        print ("ser.read",recv)
        if recv.startswith("ok "):
            break
        
    if (not recv.startswith("ok ")):
        return
    # 清空接收缓冲区
    ser.flushInput() 
    
    #2.发送字节总数
    ser.write(str(bmp_size)+"\r\n")    
    print("ser.write",str(bmp_size)+"\r\n")
    last_curl=time.time()
    recv=""
    while True:
      count = ser.inWaiting()
      nowtime=time.time()
      if nowtime-last_curl>2:
        print("show_screen timeout")
        break      
      if count != 0:
        # 读取内容并回显
        recv = ser.read(count)
        print ("ser.read",recv)
        if recv.startswith("ok "):
            break     
    if (not recv.startswith("ok ")):
        return
    # 清空接收缓冲区
    ser.flushInput()
    
    #实际发关送图片
    f = open('/home/pi/tmp.bmp', 'rb')    
    while True:
        blob_stream=f.read(1024)
        if not blob_stream:
            break
        ser.write(blob_stream)
        
        
    last_curl=time.time()
    recv=""
    while True:
      count = ser.inWaiting()
      nowtime=time.time()
      if nowtime-last_curl>2:
        print("show_screen timeout")
        break      
      if count != 0:
        # 读取内容并回显
        recv = ser.read(count)
        print ("ser.read",recv)
        if recv.startswith("ok "):
            break               
    print("recv:",recv)
        
    # 清空接收缓冲区
    ser.flushInput()        
    print("show_screen end")
  
    
def check_sock():
  #global tmp_last_msg
  if sock_server_thread.last_changed:
    #if tmp_last_msg<>sock_server_thread.last_msg:
    print("find cmd:",sock_server_thread.last_msg) 
    recv=show_screen(sock_server_thread.last_msg)
    #tmp_last_msg=sock_server_thread.last_msg
    sock_server_thread.last_changed=False


def main_loop():

  #测试效果
  show_screen(u'你好')
  
  while True:
    check_sock()
    time.sleep(1)
 

  
reload(sys)
sys.setdefaultencoding('utf8')
    
tmp_thread=sock_server_thread.do_thread()


def main():
  try:
    
    main_loop()
  except KeyboardInterrupt:
    if ser != None:
      ser.close()
    #强制中止线程stop_thread 无效
    print("中止线程")
    #利用以下技巧中止线程
    sock_server_thread.exit_now=1
    sock_client.sock_send('127.0.0.1',10090,"")


main()
