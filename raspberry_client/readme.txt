
硬件连接：
     将连接有墨水屏的esp32通过usb连接到树莓派上(例如3B)
     然后在树莓派上运行read_arduino_moshui.py,
     另开一窗口，运行python sock_client_lcd.py "今天星期几?"
     就能看到墨水屏上显示相关的文字信息.

示例代码说明:

以下代码示例用于树莓派:
1.read_arduino_moshui.py
运行:  python read_arduino_moshui.py
功能：启动端口10090端口服务器，完成:
         a. 接收 socket协议文本信息，
         b. 对收到的文本信息，处理成bmp图片, 图片传到usb连接的esp32
         c. esp32将图片展示到墨水屏.

2.sock_client_lcd.py
运行:  python sock_client_lcd.py "今天星期几?"
功能：把文本信息传给上面的端口10090端口服务器

3.loop_sock_client_lcd.py
运行:  python loop_sock_client_lcd.py
功能：每60秒调用一次 sock_client_lcd.py 发送当前日期时间的文本格式串到端口10090端口服务器