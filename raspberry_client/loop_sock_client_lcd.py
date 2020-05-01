# -*- coding: utf-8 -*-
import os  
import time  ,datetime
import sys

reload(sys)
sys.setdefaultencoding('utf8')   

#界面刷新1次平均1-3秒 
#新版本改成esp32带休眠模式，唤醒时间1秒,建议5秒发一次数据
while True:
    timestamp = datetime.datetime.now() 
    #lcd_txt= timestamp.strftime(">时钟\n%Y-%m-%d\n%H:%M:%S")
    lcd_txt= timestamp.strftime(">时钟\n%Y-%m-%d\n%H:%M")
    print("lcd show",lcd_txt)    
    os.system("python sock_client_lcd.py '"+ lcd_txt +"'")
    time.sleep(60)
    #time.sleep(5)



