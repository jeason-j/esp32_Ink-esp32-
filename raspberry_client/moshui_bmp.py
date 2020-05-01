# -*- coding: utf-8 -*-
import sys
import time
from datetime import datetime
import os

from PIL import Image
from PIL import ImageDraw
from PIL import ImageFont
import textwrap
from io import BytesIO 


#reload(sys)
#sys.setdefaultencoding('utf8')  

#屏大小:400*300
class ImgText:
  #字体36
  #字体48  每行显示: 260 / 48 = 5个字
  #        共        170 / 48 = 3行
  #字体48  每行显示: 400 / 48 = 8个字
  #        共        300 / 48 = 6行  
  #字体54  每行显示: 400 / 54 = 7个字
  #        共        300 / 54 = 5行   
  #字体62  每行显示: 400 / 62 = 6个字
  #        共        300 / 62 = 4行  
  font = ImageFont.truetype("/usr/share/fonts/truetype/wqy/wqy-zenhei.ttc",size=62)
  def __init__(self, text):
    # 预设宽度 可以修改成你需要的图片宽度
    
    # 要比画布略小,否则换行算法有问题 -20
    self.width = 360 
    # 文本
    #print("text1=",text)
    #self.text = text 
    self.text = text.replace('\\n','\n')  #处理回车
    #print("text2=",text)
    # 段落 , 行数, 行高
    self.duanluo, self.note_height, self.line_height = self.split_text()
    
  def get_duanluo(self, text):
    
    txt = Image.new('RGB', (self.width, 280), (255, 255, 255, 0))
    draw = ImageDraw.Draw(txt)
    # 所有文字的段落
    duanluo = ""
    # 宽度总和
    sum_width = 0
    # 几行
    line_count = 1
    # 行高
    line_height = 0
    for char in text:
      width, height = draw.textsize(char, ImgText.font)
      sum_width += width
      if sum_width > self.width: # 超过预设宽度就修改段落 以及当前行数
        line_count += 1
        sum_width = 0
        duanluo += '\n'
      duanluo += char
      line_height = max(height, line_height)
    if not duanluo.endswith('\n'):
      duanluo += '\n'
    return duanluo, line_height, line_count
    
  def split_text(self):
    # 按规定宽度分组
    max_line_height, total_lines = 0, 0
    allText = []
    #print("text3=",self.text)
    for text in self.text.split('\n'):
      duanluo, line_height, line_count = self.get_duanluo(text)
      print("duanluo=",duanluo)
      max_line_height = max(line_height, max_line_height)
      total_lines += line_count
      allText.append((duanluo, line_count))
    line_height = max_line_height
    total_height = total_lines * line_height
    return allText, total_height, line_height
    
  def draw_text(self):
    """
    绘图以及文字
    :return:
    """
    
    #画布大小 
    #img1 = Image.new('RGB', (240, 120), (0, 0, 0))
    img1 = Image.new('1', (400,280),1)
    draw1 = ImageDraw.Draw(img1)
    # 左上角开始
    x, y = 0, 0
    for duanluo, line_count in self.duanluo:
      #draw1.text((x, y), duanluo, fill=(255, 255, 255), font=ImgText.font)
      draw1.text((x, y), duanluo, fill=0, font=ImgText.font)
      y += self.line_height * line_count
    #转化二值化黑白      
    img1.save('/home/pi/tmp.bmp',bits=1,optimize=True)  #此图网上传输会变成非2位图片，原因不明

#生成墨水屏用的二值化 bmp
#必须加 u
# python3 moshui_bmp.py '星期三222'
txt_input=u'你好123'
if (len(sys.argv)>1):    
    txt_input=sys.argv[1]  
print("txt_input:",txt_input) 

n_txt = ImgText(txt_input)
n_txt.draw_text()
