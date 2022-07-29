import RPi.GPIO as GPIO
import board
import adafruit_dht
from time import sleep

dhtDevice = adafruit_dht.DHT11(board.D17)
FAN_PIN1 = 18
FAN_PIN2 = 27


def on_fan():
    GPIO.output(FAN_PIN1, GPIO.HIGH)
    GPIO.output(FAN_PIN2, GPIO.LOW)

def off_fan():
    GPIO.output(FAN_PIN1, GPIO.LOW)
    GPIO.output(FAN_PIN2, GPIO.LOW)


def main():
    GPIO.setmode(GPIO.BCM)
    GPIO.setwarnings(False)
    GPIO.setup(FAN_PIN1, GPIO.OUT, initial=False)
    GPIO.setup(FAN_PIN1, GPIO.OUT, initial=False)
    print("Setup FAN PIN as outputs")
    print("main() Program")

    try:
        while True:
            try:
                temp_c = dhtDevice.temperature
                humi = dhtDevice.humidity
                print(f"Temp: {temp_c:.2f} Humidity: {humi}%")
                on_fan() if temp_c > 28 or humi > 60 else off_fan()
            except RuntimeError as error:
                print(error.args[0])

            sleep(1.0)
    except KeyboardInterrupt:
        GPIO.cleanup()
    finally:
        GPIO.cleanup()


if __name__ == '__main__':
    main()
