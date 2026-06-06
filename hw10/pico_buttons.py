import time
import board
import digitalio

# Button 1 (GP14) - FLAP / jump
# Button 2 (GP15) - RESTART
# Wire each button between the GPIO pin and GND
# Internal pull-up means: unpressed = 1, pressed = 0

btn_flap    = digitalio.DigitalInOut(board.GP14)
btn_restart = digitalio.DigitalInOut(board.GP15)
btn_flap.direction    = digitalio.Direction.INPUT
btn_restart.direction = digitalio.Direction.INPUT
btn_flap.pull    = digitalio.Pull.UP
btn_restart.pull = digitalio.Pull.UP

while True:
    flap    = 0 if btn_flap.value    else 1
    restart = 0 if btn_restart.value else 1
    print("(" + str(flap) + "," + str(restart) + ")")
    time.sleep(1/60)