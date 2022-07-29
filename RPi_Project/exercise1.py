import RPi.GPIO as GPIO
from time import sleep

LED = [4, 5, 15, 14]

def main():
	print("setup GPIO PIN NUMBER & MODE")
	GPIO.setmode(GPIO.BCM)
	GPIO.setwarnings(False)
	for i in range(len(LED)):
		GPIO.setup(LED[i], GPIO.OUT)
		GPIO.output(LED[i], GPIO.LOW)

	print("try LED Control")
	try:
		while True:
			number = int(input("Input LED NUMBER (quit = q): "))
			status = input("LED SET: ")
			print(f"LED number: {number}, status: {status}")
			if number == 'q':
				break

			if status == 'ON':
				GPIO.output(LED[number - 1], GPIO.HIGH)
			elif status == 'OFF':
				GPIO.output(LED[number - 1], GPIO.LOW)
			else:
				print("ERROR!: Please input ON or OFF")
	except KeyboardInterrupt:
		print("Stop by user")
	finally:
		GPIO.cleanup()

if __name__ == '__main__':
	main()
