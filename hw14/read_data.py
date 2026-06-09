import serial
import matplotlib.pyplot as plt
import numpy as np

PORT     = '/dev/tty.usbmodem1101'   
BAUD     = 115200
N        = 400                       

ser = serial.Serial(PORT, BAUD, timeout=5)

ser.write(f'{N}\n'.encode())
print(f'Sent: collect {N} samples')

times   = []
raw     = []
filtered = []

for i in range(N):
    line = ser.readline().decode('utf-8', errors='ignore').strip()
    if not line:
        continue
    parts = line.split(',')
    if len(parts) != 3:
        continue
    times.append(float(parts[0]) / 1000.0)   
    raw.append(int(parts[1]))
    filtered.append(float(parts[2]))
    print(f'{i+1}/{N}  {line}')

ser.close()

times    = np.array(times)
raw      = np.array(raw)
filtered = np.array(filtered)


times = times - times[0]

Fs = N / times[-1]
print(f'Sample rate: {Fs:.1f} Hz')

fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(12, 8))

ax1.plot(times, raw,      'k', linewidth=0.8, label='raw')
ax1.plot(times, filtered, 'r', linewidth=1.5, label='IIR filtered')
ax1.set_xlabel('Time (s)')
ax1.set_ylabel('HX711 value')
ax1.set_title(f'Force sensor — {N} samples at {Fs:.1f} Hz')
ax1.legend()
ax1.grid(True, alpha=0.3)

def get_fft(signal, Fs):
    n   = len(signal)
    k   = np.arange(n)
    T   = n / Fs
    frq = k / T
    frq = frq[range(int(n/2))]
    Y   = np.fft.fft(signal) / n
    Y   = Y[range(int(n/2))]
    return frq, Y

frq_raw,  Y_raw  = get_fft(raw,      Fs)
frq_filt, Y_filt = get_fft(filtered, Fs)

ax2.semilogy(frq_raw,  np.abs(Y_raw),  'k', linewidth=0.8, label='raw FFT')
ax2.semilogy(frq_filt, np.abs(Y_filt), 'r', linewidth=1.5, label='filtered FFT')
ax2.set_xlabel('Frequency (Hz)')
ax2.set_ylabel('|Y(freq)|')
ax2.set_title(f'FFT — Nyquist = {Fs/2:.1f} Hz')
ax2.set_xlim([0, Fs/2])
ax2.legend()
ax2.grid(True, alpha=0.3)

plt.tight_layout()
plt.savefig('hw14_force.png', dpi=150)
plt.show()