
import RPi.GPIO as GPIO      
import sys
import time    #시간을 사용하기 위한 모듈 
import Adafruit_DHT    #온습도 센서 예제 사용을 위한 모듈
import pymysql     #데이터 베이스 사용을 위한 모듈

sensor=Adafruit_DHT.DHT22     #사용할 센서는 DHT22이다
conn=pymysql.connect(host="localhost",user="root",passwd="test",db="test")
# MySQL Connection 연결

pin=23   #GPIO 연결 핀번호

try :
    with conn.cursor() as cur :    # Connection 으로부터 cursor 생성
        sql="insert into sensor values(%s,%s,%s,%s)" 
        # sensor 테이블에 문자열 데이터 4개를 넣는다.
        while True :
            humidity,temperature=Adafruit_DHT.read_retry(sensor,pin)
            if humidity is not None and temperature is not None:
                print('Temp=%0.1f*c Humidity=%0.1f'%(temperature,humidity))
                #측정된 온도와 습도를 소수 한 자리까지 출력
                cur.execute(sql,
                        ('DHT22',time.strftime("%Y-%m-%d %H:%M:%S",time.localt                           ime()), temperature,humidity))
                # SQL 실행하기 데이터베이스에 ‘DHT22’,현재시간,온도,습도 저장
                conn.commit()  # 실행 mysql 서버에 확정 반영하기   
            else:
                print("fail")
            time.sleep(1)

except KeyboardInterrupt:   # Ctrl+C 입력시 예외 발생
    exit()    #종료
finally:
    conn.close()  # Connection 닫기
