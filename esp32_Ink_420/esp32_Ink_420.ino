#include "SPIFFS.h" 
#include <GxEPD.h>

//400X300 全刷3s,局刷0.3s
#include <GxGDEW042T2/GxGDEW042T2.h>      // 4.2" b/w

//24点阵英文字体文件
#include <Fonts/FreeMonoBold24pt7b.h>

#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>

GxIO_Class io(SPI, /*CS=5*/ SS, /*DC=*/17, /*RST=*/ 16); // arbitrary selection of 17, 16
GxEPD_Class display(io, /*RST=*/ 16, /*BUSY=*/ 4); // arbitrary selection of (16), 4


const GFXfont* font24 = &FreeMonoBold24pt7b;

//bmp显示用的变量
static const uint16_t input_buffer_pixels = 20;       // may affect performance
static const uint16_t max_palette_pixels = 256;       // for depth <= 8
uint8_t mono_palette_buffer[max_palette_pixels / 8];  // palette buffer for depth <= 8 b/w
uint8_t color_palette_buffer[max_palette_pixels / 8]; // palette buffer for depth <= 8 c/w
uint8_t input_buffer[3 * input_buffer_pixels];        // up to depth 24

//bmp二位黑白图像
//400*280的图像一般大小约 14K大小（注意：用1位色深，如果是8位色深，16位色深，则大小会翻多倍，显示缓存就会不够用）

//注意：bmp缓存最多不能超过100k， 申请内存时正常，但运行时会异常，重启，所以不要超过100k
//如果用2位色深的bmp, 50K 用于普通显示屏基本够用
unsigned int img_buff_max = 50 * 1024;
byte* img_buff;
unsigned int img_buff_p = 0;

//读取图像缓存片断 (uart串口每批次采集字节数随机
//1024长度足够
byte* tmp_img_buff;
unsigned int tmp_img_buff_p = 0;

//文件接收放弃标志,防止接收数据错误无限接收文件
bool file_bad = false;
//文件接收开始时间，用于计算接收一张bmp总时长
uint32_t file_starttime = 0;
//找到bmp文件标志头标志
bool find_head_flag = false;

//bmp接收状态标志
int bmp_state = 0; //0 无状态 1 收到到bmp文件头  2 接收bmp状态
int bmp_filesize = 0;  //预期bmp文件字节数
unsigned int last_state_time = 0;  //最后一次状态改变时间 (防止接收中断,程序自动复位状态位用)



/*
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
     
*/






void setup()
{
  // Serial.begin(115200); //速度太低,调试用
  Serial.begin(921600);  // 传输图片必须用这个高速度波特率,否则每张图片传输时间太长

  //初始化墨水屏
  display.init(0);

  //display.setRotation(3);

  //旋转屏 0,1,2,3 分别累进90度
  display.setRotation(0);
  //清屏 
  display.fillScreen(GxEPD_WHITE);
  //全刷刷屏
  //只有到这一步屏幕才开始刷新，连续闪烁3秒，感觉极不好，
  //尽量用能支持局刷的屏.局刷的速度与刷屏面积几乎关系不大，一般0.3秒刷完屏, 
  //有的屏局刷显示效果不好,例如字迹模糊，或有重影，一般勉强可接受。 有的屏局刷效果和全刷一样。建议找商家问清楚
  display.update();

  display.setTextColor(GxEPD_BLACK);
  display.setFont(font24);

  //初始化SPIFFS, 接收bmp时会用到此区存放临时bmp
  if (!SPIFFS.begin()) {
    Serial.println("SPIFFS initialisation failed!");
    while (1) yield(); // Stay here twiddling thumbs waiting
  }

  //接收缓存1, 接收一批字节，例如14K的bmp数据，每次可能传过来200字节，等一会再收到数据
  tmp_img_buff = (byte*)malloc(1024);
  tmp_img_buff_p = 0;

  //接收缓存2, 存放收到的完整bmp, 4.2寸屏最多10几k大小
  //缓存大小不是无限的，如果文件太大，可以边接收边存入SPIFFS的临时文件，
  //SPIFFS 写入速度有点慢，如果写入速度低于串口接收速度，可能造成数据丢失，用缓存可以提高可靠性
  img_buff = (byte*)malloc(img_buff_max);
  img_buff_p = 0;
  //Serial.println("start");


  //测试,显示一张bmp到墨水屏
  //必须先执行 arduino/ESP32 Sketch Data upload把 data目录文件传到esp32,否则会找不到此文件 qr.bmp
  char *  tmpfilefn = "/qr.bmp";
  if (SPIFFS.exists(tmpfilefn))
  {
    drawBitmap(tmpfilefn, 0, 0, false);
    //display.update();
    display.updateWindow(0, 0, GxEPD_WIDTH , GxEPD_HEIGHT , true);
  }



  //2秒，清串口缓存
  //过滤掉单片机启动时自动发来的无用串口数据，特别是esp32客户端，刚启动时会发一些无用数据
  uint32_t starttime = 0;
  char ch;
  starttime = millis() / 1000 ;
  //delay(1000);
  while (true)
  {
    if (millis() / 1000 - starttime > 2)
      break;
    while (Serial.available())
    {
      ch = (char)Serial.read();
    }
    delay(20);
    yield();
  }

  /*
    char ch ;
    while (Serial.available())
    {
     ch = (char)Serial.read();
    }
  */

}



//查找出现连续b1,b2的位置
//找bmp文件头标志字节
unsigned int  findbyte_index(const byte arrayname[],  int array_size, byte b1, byte b2)
{
  //byte * array_p= arrayname;
  int  find_index = -1;
  for ( int loop1 = 0; loop1 < array_size - 1; loop1++)
  {
    if (arrayname[loop1] == b1 && arrayname[loop1 + 1] == b2)
    {
      find_index = loop1;
      break;
    }
  }
  return find_index;
}

void loop()
{
  String line;

  //10秒没完成一轮图像接收，重置状态
  //防止接收数据中有中断，程序进入某循环无限挂起
  //程序如太复杂，可考虑用dog
  if (bmp_state > 0 && ( millis() / 1000 - last_state_time) > 10)
  {
    bmp_state = 0;
    last_state_time = millis() / 1000;
  }


  //原始状态
  if (bmp_state == 0)
  {
    while (Serial.available()) {
      line = Serial.readStringUntil('\n');
    }
    if (line.length() > 0 && line.startsWith("jpg>"))
    {
      //实收到5字符  (说明丢掉了\n  \r\n )
      Serial.println("ok 1 " + String( line.length()) + " " + line);
      bmp_state = 1;
      last_state_time = millis() / 1000;
      line = "";
    }
  }


  //标志收到状态
  if (bmp_state == 1)
  {
    while (Serial.available())
    {
      line = Serial.readStringUntil('\n');
    }

    if ( line.length() > 0 )
    {
      //12315 实收到6字符  (说明丢掉了\n  \r\n )
      Serial.println("ok 2 " + String( line.length()) + " " + line);
      bmp_filesize = line.toInt();
      if (bmp_filesize > 0)
      {
        bmp_state = 2;
        last_state_time = millis() / 1000;
      }
      line = "";
    }
  }

  if (bmp_state == 2)
  {
    byte ch;
    byte* buff_tmp;

    buff_tmp = tmp_img_buff;

    uint32_t starttime = 0;
    bool tmp_buff_full = false;

    starttime = millis() ;
    //读入一批数据 (每一次只读115字节？)
    while (Serial.available()) {
      //Serial.println("#");
      //0.1秒可以连续
      if  (tmp_buff_full or (millis() - starttime > 100))
        break;

      //临时接收缓存，防止无限等待
      while (Serial.available())
      {
        ch = Serial.read();
        *buff_tmp = ch;
        tmp_img_buff_p = tmp_img_buff_p + 1;
        buff_tmp = tmp_img_buff + tmp_img_buff_p;
        starttime = millis() ;

        //接收缓存满
        if (tmp_img_buff_p >= 1024)
        {
          tmp_buff_full = true;
          break;
        }
      }
      //delay(5);
      yield();
      //连续多次接收数据，直到无数据
    }

    //检查bmp头

    //本批数据分析
    if (tmp_img_buff_p > 0)
    {
      //检查jpeg头
      int  bmp_begin_index = -1;

      //找到文件头，就不再查找了, bmp不同于jpg,标志位有可能在文件中存在
      if (find_head_flag == false)
        bmp_begin_index = findbyte_index(tmp_img_buff, tmp_img_buff_p, 0x42, 0x4d);

      //1.发现bmp头

      if (bmp_begin_index > -1)
      {
        Serial.println("find bmp head:" + String(bmp_begin_index));
        file_starttime = millis() ;
        img_buff_p = 0;
        memcpy(img_buff + img_buff_p, tmp_img_buff + bmp_begin_index, tmp_img_buff_p - bmp_begin_index);
        img_buff_p = img_buff_p +  tmp_img_buff_p - bmp_begin_index;
        file_bad = false;  //清空上次文件丢弃标志
        find_head_flag = true;
      }
      //2.文件中或文尾
      else if  (find_head_flag )
      {

        if (file_bad == false)
        {
          if (img_buff_p + tmp_img_buff_p <= img_buff_max)
          {
            memcpy(img_buff + img_buff_p, tmp_img_buff, tmp_img_buff_p);
            img_buff_p = img_buff_p + tmp_img_buff_p;

            //判断图像文件是否接收完
            if (img_buff_p >= bmp_filesize)
            {
              Serial.println("drawbmp size=" + String(bmp_filesize));
              drawbmp();
              //局刷，速度快,显示效果不如全局效果好
              //注意：如果屏不显示内容，或显示效果差，把此句改成全刷！！！
              display.updateWindow(0, 0, GxEPD_WIDTH , GxEPD_HEIGHT , true);
              // display.update();
              Serial.println("ok 3 drawbmp size=" + String(bmp_filesize));
              //清零
              img_buff_p = 0;
              find_head_flag = false;
              //还原bmp接收状态
              bmp_state = 0;
              bmp_filesize = 0;
            }
          }
          else
            file_bad = true;
        }
      }

      //清空本次数据
      tmp_img_buff_p = 0;
    }
  }


}


//画图
//把bmp接收缓存区数据存入SPIFFS临时文件tmp.bmp
//显示tmp.bmp到 墨水屏
//注：可省掉一步，直接把bmp接收缓存区数据解析到墨水屏,可改进
void drawbmp()
{
  char *  tmpfilefn = "/tmp.bmp";
  if (SPIFFS.exists(tmpfilefn))
  {
    SPIFFS.remove(tmpfilefn);
    Serial.println(String(tmpfilefn) + " remove");
  }

  File bmpFS = SPIFFS.open(tmpfilefn, "w");
  bmpFS.write(img_buff, img_buff_p);
  bmpFS.close();
  drawBitmap(tmpfilefn, 0, 0, false);

}

//网上抄来的算法，bmp文件 ==>墨水屏
void drawBitmap(const char *filename, int16_t x, int16_t y, bool with_color)
{
  File file;
  bool valid = false; // valid format to be handled
  bool flip = true;   // bitmap is stored bottom-to-top
  uint32_t startTime = millis();
  if ((x >= display.width()) || (y >= display.height()))
    return;
  Serial.println();
  Serial.print("Loading image '");
  Serial.print(filename);
  Serial.println('\'');

  file = SPIFFS.open(filename, FILE_READ);
  if (!file) {
    Serial.print("File not found");
    return;
  }

  // Parse BMP header
  if (read16(file) == 0x4D42) {
    // BMP signature
    uint32_t fileSize = read32(file);
    uint32_t creatorBytes = read32(file);
    uint32_t imageOffset = read32(file); // Start of image data
    uint32_t headerSize = read32(file);
    uint32_t width = read32(file);
    uint32_t height = read32(file);
    uint16_t planes = read16(file);
    uint16_t depth = read16(file); // bits per pixel
    uint32_t format = read32(file);
    if ((planes == 1) && ((format == 0) || (format == 3))) {
      // uncompressed is handled, 565 also
      Serial.print("File size: ");
      Serial.println(fileSize);
      Serial.print("Image Offset: ");
      Serial.println(imageOffset);
      Serial.print("Header size: ");
      Serial.println(headerSize);
      Serial.print("Bit Depth: ");
      Serial.println(depth);
      Serial.print("Image size: ");
      Serial.print(width);
      Serial.print('x');
      Serial.println(height);
      // BMP rows are padded (if needed) to 4-byte boundary
      uint32_t rowSize = (width * depth / 8 + 3) & ~3;
      if (depth < 8)
        rowSize = ((width * depth + 8 - depth) / 8 + 3) & ~3;
      if (height < 0) {
        height = -height;
        flip = false;
      }
      uint16_t w = width;
      uint16_t h = height;
      if ((x + w - 1) >= display.width())
        w = display.width() - x;
      if ((y + h - 1) >= display.height())
        h = display.height() - y;
      valid = true;
      uint8_t bitmask = 0xFF;
      uint8_t bitshift = 8 - depth;
      uint16_t red, green, blue;
      bool whitish, colored;
      if (depth == 1)
        with_color = false;
      if (depth <= 8) {
        if (depth < 8)
          bitmask >>= depth;
        file.seek(54); //palette is always @ 54
        for (uint16_t pn = 0; pn < (1 << depth); pn++) {
          blue = file.read();
          green = file.read();
          red = file.read();
          file.read();
          whitish = with_color ? ((red > 0x80) && (green > 0x80) && (blue > 0x80)) : ((red + green + blue) > 3 * 0x80); // whitish
          colored = (red > 0xF0) || ((green > 0xF0) && (blue > 0xF0));                                                  // reddish or yellowish?
          if (0 == pn % 8)
            mono_palette_buffer[pn / 8] = 0;
          mono_palette_buffer[pn / 8] |= whitish << pn % 8;
          if (0 == pn % 8)
            color_palette_buffer[pn / 8] = 0;
          color_palette_buffer[pn / 8] |= colored << pn % 8;
        }
      }

      //display.fillScreen(GxEPD_WHITE);
      //上句太费时间
      display.fillRect(0, 0, GxEPD_WIDTH, GxEPD_HEIGHT,   GxEPD_WHITE);

      uint32_t rowPosition = flip ? imageOffset + (height - h) * rowSize : imageOffset;
      for (uint16_t row = 0; row < h; row++, rowPosition += rowSize) {
        // for each line
        uint32_t in_remain = rowSize;
        uint32_t in_idx = 0;
        uint32_t in_bytes = 0;
        uint8_t in_byte = 0; // for depth <= 8
        uint8_t in_bits = 0; // for depth <= 8
        uint16_t color = GxEPD_WHITE;
        file.seek(rowPosition);
        for (uint16_t col = 0; col < w; col++) {
          // for each pixel
          // Time to read more pixel data?
          if (in_idx >= in_bytes) {
            // ok, exact match for 24bit also (size IS multiple of 3)
            in_bytes = file.read(input_buffer, in_remain > sizeof(input_buffer) ? sizeof(input_buffer) : in_remain);
            in_remain -= in_bytes;
            in_idx = 0;
          }
          switch (depth) {
            case 24:
              blue = input_buffer[in_idx++];
              green = input_buffer[in_idx++];
              red = input_buffer[in_idx++];
              whitish = with_color ? ((red > 0x80) && (green > 0x80) && (blue > 0x80)) : ((red + green + blue) > 3 * 0x80); // whitish
              colored = (red > 0xF0) || ((green > 0xF0) && (blue > 0xF0));                                                  // reddish or yellowish?
              break;
            case 16: {
                uint8_t lsb = input_buffer[in_idx++];
                uint8_t msb = input_buffer[in_idx++];
                if (format == 0) {
                  // 555
                  blue = (lsb & 0x1F) << 3;
                  green = ((msb & 0x03) << 6) | ((lsb & 0xE0) >> 2);
                  red = (msb & 0x7C) << 1;
                } else {
                  // 565
                  blue = (lsb & 0x1F) << 3;
                  green = ((msb & 0x07) << 5) | ((lsb & 0xE0) >> 3);
                  red = (msb & 0xF8);
                }
                whitish = with_color ? ((red > 0x80) && (green > 0x80) && (blue > 0x80)) : ((red + green + blue) > 3 * 0x80); // whitish
                colored = (red > 0xF0) || ((green > 0xF0) && (blue > 0xF0));                                                  // reddish or yellowish?
              }
              break;
            case 1:
            case 4:
            case 8: {
                if (0 == in_bits) {
                  in_byte = input_buffer[in_idx++];
                  in_bits = 8;
                }
                uint16_t pn = (in_byte >> bitshift) & bitmask;
                whitish = mono_palette_buffer[pn / 8] & (0x1 << pn % 8);
                colored = color_palette_buffer[pn / 8] & (0x1 << pn % 8);
                in_byte <<= depth;
                in_bits -= depth;
              }
              break;
          }
          if (whitish) {
            color = GxEPD_WHITE;
          } else if (colored && with_color) {
            color = GxEPD_RED;
          } else {
            color = GxEPD_BLACK;
          }
          uint16_t yrow = y + (flip ? h - row - 1 : row);
          display.drawPixel(x + col, yrow, color);
        } // end pixel
      }     // end line
      Serial.print("loaded in ");
      Serial.print(millis() - startTime);
      Serial.println(" ms");
    }
  }
  file.close();
  if (!valid) {
    Serial.println("bitmap format not handled.");
  }
}

//drawBitmap 函数专用
uint16_t read16(File &f)
{
  // BMP data is stored little-endian, same as Arduino.
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

//drawBitmap 函数专用
uint32_t read32(File &f)
{
  // BMP data is stored little-endian, same as Arduino.
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}
