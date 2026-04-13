import time
import board
import pwmio
from adafruit_motor import servo

# GP16 = pin 21 on Pico 2
pwm = pwmio.PWMOut(board.GP16, duty_cycle=0, frequency=50)
my_servo = servo.Servo(pwm, min_pulse=1000, max_pulse=2000)

print("Servo sweep starting...")

while True:
    # Sweep 0 → 180
    for angle in range(0, 181, 1):
        my_servo.angle = angle
        time.sleep(0.015)
    # Sweep 180 → 0
    for angle in range(180, -1, -1):
        my_servo.angle = angle
        time.sleep(0.015)