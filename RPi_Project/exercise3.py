import RPi.GPIO as GPIO
from time import sleep
import spidev

spi = spidev.SpiDev()
CDS_CHANNEL = 0
LED = [4, 5, 15, 14]

def initMcp3208():
    spi.open(0, 0) 
    spi.max_speed_hz = 1000000
    spi.mode = 3

def buildReadCommand(channel):

    startBit = 0x04
    singleEnded = 0x08

    configBit = [startBit | ((singleEnded | (channel & 0x07)) >> 2), (channel & 0x07) << 6, 0x00]

    return configBit

def processAdcValue(result):
    byte2=(result[1] & 0x0F)
    return (byte2 << 8) | result[2]

def analogRead(channel):
    if (channel>7) or (channel<0):
        return -1

    r = spi.xfer2(buildReadCommand(channel))
    adc_out = processAdcValue(r)
    return adc_out

def controlMcp3208(channel):
    analogVal = analogRead(channel)
    return analogVal

def readSensor(channel):
    return controlMcp3208(channel)

def main():
    print("setup GPIO PIN NUMBER & MODE")
    GPIO.setmode(GPIO.BCM)
    GPIO.setwarnings(False)
    GPIO.setup(LED, GPIO.OUT, initial=False)

    
    initMcp3208()
    print("Setup pin as outputs")

    try:
        while True:
            readVal = readSensor(CDS_CHANNEL)

            voltage = readVal * 4.096 / 4096
            if voltage <1 and voltage >=0 :
                GPIO.output(LED[:], GPIO.HIGH)

            elif voltage <2 and voltage >=1:
                GPIO.output(LED[:3], GPIO.HIGH)
                GPIO.output(LED[3:], GPIO.LOW)
            elif voltage <3 and voltage >=2:
                GPIO.output(LED[:2], GPIO.HIGH)
                GPIO.output(LED[2:], GPIO.LOW)
            elif voltage <4 and voltage >=3:
                GPIO.output(LED[:1], GPIO.HIGH)
                GPIO.output(LED[1:], GPIO.LOW)
            else:
                GPIO.output(LED, GPIO.LOW)
            
            print("CDS Val=%d\tVoltage=%f" % (readVal, voltage))
            sleep(0.5)
            
    except KeyboardInterrupt:
        print("Stop by user")
        spi.close()
        GPIO.cleanup()     

if __name__ == '__main__':
    main()