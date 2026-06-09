import serial
import matplotlib.pyplot as plt

ser = serial.Serial(
    "/dev/tty.usbmodem1101",
    115200,
    timeout=1
)

angles = []
forces = []

while len(angles) < 500:

    try:

        line = ser.readline().decode().strip()

        a,f = line.split(",")

        angles.append(float(a))
        forces.append(int(f))

    except:
        pass

plt.figure()

plt.plot(
    angles,
    forces
)

plt.xlabel("Displacement (deg)")
plt.ylabel("Force (raw HX711)")
plt.title("Force vs Displacement")

plt.grid()

plt.show()