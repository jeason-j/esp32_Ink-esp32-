esp32串口接收bmp图像数据并显示到墨水屏
树莓派通过usb串口连接到esp32, 然后树莓派运行程序把任意文字显示到墨水屏
好处：显示汉字，树莓派显示屏


 增强型墨水屏
  esp32串口接收bmp图像数据并显示到墨水屏
  
  一.硬件:
  1.esp32墨水屏驱动板
         https://item.taobao.com/item.htm?spm=a1z1r.7974869.0.0.269c3ad4sJNvrL&id=608962598880
         T5 V2.4 版本       
     硬件资料：
         https://github.com/Xinyuan-LilyGO/T5-Ink-Screen-Series
         https://github.com/lewisxhe/TTGO-EPaper-Series       
  2.电子墨水屏：
     4.2寸屏
         https://item.taobao.com/item.htm?spm=a1z10.5-c.w4002-1995619223.11.19fe1061mPBsST&id=533910866519
     硬件资料:
         4.2寸屏 资料
         http://www.e-paper-display.cn/products_detail/productId=333.html
     太小的屏显示内容受限，太大的屏价格贵
     注意：支持局刷很重要，全刷刷屏时效果太差。
  二.接线
     墨水屏排线接到esp32墨水屏驱动板
  三.编译：
  1.必备 arduino 软件 最新版 1.8.12
  2.库:  arduino for esp32 (来源 GitHub)
         https://github.com/espressif/arduino-esp32
         GxEPD (来源 GitHub)
         https://github.com/ZinggJM/GxEPD
  3.esp32上传文件到 esp32 SPIFFS分区的软件
         https://github.com/me-no-dev/arduino-esp32fs-plugin
  3.编译：
         开发板: esp32 dev module,
         Partition Schemo: Default 4Mb with spiffs (1.2MB APP/ 1.5Mb SPIFFS)
         PSRAM: 随意，如果esp32板没有PSRAM硬件就不要选Enabled
         选择好端口，执行编译上传
         arduino 菜单 工具/ESP32 Sketch Data upload把示例bmp图传到esp32
  四.代码说明
  交互协议步骤
  1.收到 jpg>\r\n   串口返回 ok 进入bmp准备接收状态
  2.收到 字节长度字串\r\n 串口返回 ok 获取到bmp文件大小
  3.接收 bmp 数据文件到缓存  
  4.当收到字节数等于第二步得到的字节数后结束，将缓存数据存入 spiffs 文件名tmp.bmp
  5.将tmp.bmp文件解析成点阵数据，输出点阵图片到墨水屏
  处理速度
  1. bmp图像 400*280 约14k字节 (注意是2位色深， 不是8位色深，甚至24色深)
     921600串口速度，用时120ms秒   14000*8/921600=0.12秒
  2.bmp显示图像约170ms
  共用时: 290ms
  四.用电
  esp32: 60ma (可考虑在接收bmp完毕后esp32进入休眠模式，当收到串口信息后自动唤醒，待机电流可降到15ma,节能版本基本开发完)
  墨水屏: 工作状态约10-20ma,工作完自动进入休眠,体眠状态用电几乎为0
  合计：刷屏时75ma, 非刷屏60ma （注：没启用wifi）
  五.其它资料:           
     http://www.waveshare.net/wiki/4.2inch_e-Paper_Module 
     用arduino/树莓派 驱动 墨水屏 介绍怎么用arduino,树莓派驱动墨水屏
     https://github.com/ZinggJM/GxEPD
     用GxEPD驱动 墨水屏 显示，示例很清楚
