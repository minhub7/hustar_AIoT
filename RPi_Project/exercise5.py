import RPi.GPIO as GPIO
import board
import digitalio
import adafruit_character_lcd.character_lcd as character_lcd
import adafruit_dht
from time import sleep
import GPIO_EX

# Raspberry PI pin setup for LCD
dhtDevice = adafruit_dht.DHT11(board.D17)
lcd_rs = digitalio.DigitalInOut(board.D22)
lcd_en = digitalio.DigitalInOut(board.D24)
lcd_d7 = digitalio.DigitalInOut(board.D21)
lcd_d6 = digitalio.DigitalInOut(board.D26)
lcd_d5 = digitalio.DigitalInOut(board.D20)
lcd_d4 = digitalio.DigitalInOut(board.D19)

lcd_columns = 16
lcd_rows = 2
lcd = character_lcd.Character_LCD_Mono(lcd_rs, lcd_en, lcd_d4, lcd_d5, lcd_d6, lcd_d7, lcd_columns, lcd_rows)

# Setup for keypad
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


# Define function for Control Keypad
def initKeypad():
    for i in range(0, COL_NUM):
        GPIO_EX.setup(colTable[i], GPIO_EX.IN)
    for i in range(0, ROW_NUM):
        GPIO_EX.setup(rowTable[i], GPIO_EX.OUT)


def selectRow(rowNum):
    for i in range(0, ROW_NUM):
        if rowNum == (i + 1):
            GPIO_EX.output(rowTable[i], GPIO_EX.HIGH)
            sleep(0.001)
        else:
            GPIO_EX.output(rowTable[i], GPIO_EX.LOW)
            sleep(0.001)
    return rowNum


def readCol():
    keypadstate = -1
    for i in range(0, COL_NUM):
        inputKey = GPIO_EX.input(colTable[i])  # ������ ��ư�� �ּҰ��� �Է�
        if inputKey:  # Ű�� �Է��� �Ǹ�
            keypadstate = keypadstate + (i + 2)  # keypadstate = -1 +2 == 1
            sleep(0.5)
    return keypadstate  # �� ������ ��ȯ


def readKeypad():
    global g_preData
    keyData = -1  # keyData: ������ ��ư

    for i in range(1, ROW_NUM + 1):
        runningStep = selectRow(i)  # runningStep = 1
        rowData = readCol()  # rowData = ������ col ��ȣ ���� if 1��° ���� �������� => rowData = 1
        # print(f"i is {i}, rowData is {rowData}")
        selectRow(0)  # ù��° ������ ��������
        sleep(0.001)

        if rowData != -1:
            if i == 1:  # rowData�� ������ �Ǹ�
                keyData = keypad[rowData]  # keyData�� �� ��ȣ�� ����
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

# Define function for control LCD
def initTextlcd():
    lcd.clear()
    lcd.home()
    lcd.cursor_position(0, 0)
    sleep(1.0)

def displayText(text='', col=0, row=0):
    lcd.cursor_position(col, row)
    lcd.message = text

def clearTextlcd():
    lcd.clear()
    lcd.message = 'clear LCD\nGoodbye!'
    sleep(2.0)
    lcd.clear()

def password_input():
    count = 0
    passwd = ''
    while True:
        if count == 4:
            break

        keyData = readKeypad()
        if keyData != -1:
            passwd += str(keyData)
            count += 1

    return passwd

def main():
    print("main() Program")
    GPIO.setmode(GPIO.BCM)
    GPIO.setwarnings(False)

    print("Init Keypad & Textlcd")
    initKeypad()
    initTextlcd()

    try:
        lcd.clear()
        displayText("input password!", 0, 0)
        sleep(2.0)

        passwd1 = password_input()

        lcd.clear()
        displayText(passwd1, 0, 0)
        sleep(3.0)

        lcd.clear()
        displayText("confirm password!", 0, 0)
        sleep(2.0)

        passwd2 = password_input()

        lcd.clear()
        displayText(passwd2, 0, 0)

        if passwd1 == passwd2:
            displayText("CORRECT", 0, 1)
            sleep(2.0)
            while True:
                temp_c = dhtDevice.temperature
                humi = dhtDevice.humidity

                print(f"Temp: {temp_c:.2f} Humidity: {humi}%")
                temp_str = f"Temp: {temp_c:.2f}'C"
                humi_str = f"Humidity: {humi}%"

                lcd.clear()
                displayText(temp_str, 0, 0)
                displayText(humi_str, 0, 1)
                sleep(2.0)
        else:
            displayText("FAIL", 0, 1)
        sleep(3.0)


    except KeyboardInterrupt:
        GPIO.cleanup()
        lcd.clear()
    finally:
        GPIO.cleanup()
        clearTextlcd()


if __name__ == '__main__':
    main()
