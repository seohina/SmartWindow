import time 
import os
import RPi.GPIO as GPIO
import pandas as pd
from sqlalchemy import create_engine
from PIL import Image
import base64
from io import BytesIO
import pymysql
import sys
import datetime
# mysql에 연결한다
conn=pymysql.connect(host="localhost",user="root",passwd="test",db="test")

GPIO.setmode(GPIO.BOARD)
# 아웃풋을 24번핀에 연결한다.
pirPin = 24

GPIO.setup(pirPin, GPIO.IN)

try :
    with conn.cursor() as cur :
# mysql 데이터베이스 safe 테이블에 문자열 3개를 넣는다
        sql="insert into safe values(%s,%s,%s)" 
# while문을 반복한다
        while True: 
# 외부인이 감지되면
                if GPIO.input(pirPin)==GPIO.LOW:
# motion detected!를 출력한다.
                        print("motion detected!")
                        now=datetime.datetime.now()
                        filename=now.strftime('%H%M%S')
# 실시간으로 사진을 찍는다.
                        os.system('fswebcam -r 320x240 -S 3 --jpeg 50 --save /home/pi/filename.jpg')

                        engine =create_engine('mysql+pymysql://root:test@localhost/test',echo=False)
                        buffer=BytesIO()
# 사진을 jpg로 저장한다.
                        im=Image.open('filename'+'.jpg')
                        im.save(buffer,format='jpeg')
# jpg로된 사진을 blob로 변환한다.
                        img_str=base64.b64encode(buffer.getvalue())
                        print(img_str)
                        img_df=img_str
# 테이블에 시간, detected!, 사진을 저장한다.
                        cur.execute(sql,(time.strftime("%Y-%m-%d %H:%M:%S",time.localtime()),"detected!",img_df));
                        conn.commit(); # 실행 mysql 서버에 확정 반영하기
                        time.sleep(5)
# 외부인 감지가 없으면 no motion 문자열을 출력한다.
 else:
                        print("no motion")
                        time.sleep(3)
# 테이블에 시간, no motion, “”을 저장한다.
                        cur.execute(sql,(time.strftime("%Y-%m-%d %H:%M:%S",time.localtime()),"no motion",""));
                        conn.commit(); # 실행 mysql 서버에 확정 반영하기
                        time.sleep(3)
except KeyboardInterrupt: # ctrl+c 입력시 예외발생
    exit() #종료 
finally:
    conn.close() # mysql 연결을 끊는다.
