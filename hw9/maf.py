from plot import load_csv
from fft import get_fft   

import numpy as np
import matplotlib.pyplot as plt

def moving_average(signal, X):
    filtered = []
    for i in range(len(signal)):
        if i < X:
            filtered.append(sum(signal[:i+1]) / (i+1))
        else:
            window = signal[i-X+1 : i+1]
            filtered.append(sum(window) / X)
    return filtered

files = ['sigA.csv', 'sigB.csv', 'sigC.csv', 'sigD.csv']
best_X = {'sigA.csv': 500, 'sigB.csv': 200, 'sigC.csv': 10, 'sigD.csv': 40} 

fig, axes = plt.subplots(4, 2, figsize=(12, 16))

for row, fname in enumerate(files):
    t, s = load_csv(fname)
    t = np.array(t)
    s = np.array(s)

    X = best_X[fname]
    filtered = moving_average(s, X)
    filtered = np.array(filtered)

    Fs = len(t) / t[-1]
    frq_raw,  Y_raw  = get_fft(s,        Fs)
    frq_filt, Y_filt = get_fft(filtered, Fs)

    ax1 = axes[row, 0]
    ax2 = axes[row, 1]

    ax1.plot(t, s,        'k', linewidth=0.8, label='raw')
    ax1.plot(t, filtered, 'r', linewidth=1.5, label=f'MAF X={X}')
    ax1.set_xlabel('Time')
    ax1.set_ylabel('Amplitude')
    ax1.set_title(f'{fname} — MAF X={X}')
    ax1.legend()

    ax2.loglog(frq_raw,  abs(Y_raw),  'k', linewidth=0.8, label='raw FFT')
    ax2.loglog(frq_filt, abs(Y_filt), 'r', linewidth=1.5, label='filtered FFT')
    ax2.set_xlabel('Freq (Hz)')
    ax2.set_ylabel('|Y(freq)|')
    ax2.set_title(f'{fname} — FFT comparison')
    ax2.legend()

plt.tight_layout()
plt.savefig('maf_filtered.png', dpi=150)
plt.show()