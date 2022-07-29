# -*- coding: utf-8 -*-
import RPi.GPIO as GPIO
from time import sleep
import GPIO_EX

LED = [4, 5, 15, 14]
LED_status = [False, False, False, False]

ROW0_PIN = 0
ROW1_PIN = 1
ROW2_PIN = 2
ROW3_PIN = 3
COL0_PIN = 4
COL1_PIN = 5
COL2_PIN = 6

COL_NUM = 3
ROW_NUM = 4

g_preData = 0

colTable = [COL0_PIN, COL1_PIN, COL2_PIN]
rowTable = [ROW0_PIN, ROW1_PIN, ROW2_PIN, ROW3_PIN]
keypad = [-1, 1, 2, 3, 4, 5, 6, 7, 8, 9, '*', 0, '#']


def initKeypad():
    for i in range(0, COL_NUM):
        GPIO_EX.setup(colTable[i], GPIO_EX.IN)
    for i in range(0, ROW_NUM):
        GPIO_EX.setup(rowTable[i], GPIO_EX.OUT)


def selectRow(rowNum):
    for i in range(0, ROW_NUM):
        if rowNum == (i+1):
            GPIO_EX.output(rowTable[i], GPIO_EX.HIGH)
            sleep(0.001)
        else:
            GPIO_EX.output(rowTable[i], GPIO_EX.LOW)
            sleep(0.001)
    return rowNum

def readCol():
    keypadstate = -1
    for i in range(0, COL_NUM):
        inputKey = GPIO_EX.input(colTable[i])   # 눌려진 버튼의 주소값을 입력
        if inputKey:    # 키가 입력이 되면
            keypadstate = keypadstate + (i+2)   # keypadstate = -1 +2 == 1
            sleep(0.5)
    return keypadstate  # 몇 열인지 반환

def readKeypad():
    global g_preData
    keyData = -1    # keyData: 눌려진 버튼

    for i in range(1, ROW_NUM + 1):
        runningStep = selectRow(i)  # runningStep = 1
        rowData = readCol()    # rowData = 눌려진 col 번호 받음 if 1번째 열이 눌렸으면 => rowData = 1
        # print(f"i is {i}, rowData is {rowData}")
        selectRow(0)  # 첫번째 행으로 돌려놓음
        sleep(0.001)

        if rowData != -1:
            if i == 1:    # rowData가 선택이 되면
                keyData = keypad[rowData]  # keyData는 열 번호로 저장
            elif i == 2:
                keyData = keypad[rowData + 3]
            elif i == 3:
                keyData = keypad[rowData + 6]
            else:
                keyData = keypad[rowData + 9]
            break

        sleep(0.5)
        
    if keyData == -1:
        return -1

    if g_preData == keyData:
        g_preData = -1
        return -1

    g_preData = keyData

    print(f"\r\nKeypad Data : {keyData}")

    return keyData
    
def main():
    GPIO.setwarnings(False)
    GPIO.setmode(GPIO.BCM)

    print("LED setup start")
    for i in range(len(LED)):
        GPIO.setup(LED[i], GPIO.OUT)
        GPIO.output(LED[i], GPIO.LOW)
    
    initKeypad()
    print("setup keypad pin")
    try:
        while True:
            keyData = readKeypad()
            if keyData in keypad[1:5]:
                LED_status[keyData - 1] = not LED_status[keyData - 1]
                GPIO.output(LED[keyData - 1], LED_status[keyData - 1])
            elif keyData in keypad[5:]:
                for i in range(len(LED)):
                    GPIO.output(LED[i], GPIO.LOW)

    except KeyboardInterrupt:
        GPIO.cleanup()
    finally:
        GPIO.cleanup()

if __name__ == '__main__':
    main()