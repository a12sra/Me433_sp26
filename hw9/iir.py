from plot import load_csv
from fft import get_fft

import numpy as np
import matplotlib.pyplot as plt

def iir_filter(signal, A):
    B = 1.0 - A
    filtered = [signal[0]]
    for i in range(1, len(signal)):
        new_val = A * filtered[i-1] + B * signal[i]
        filtered.append(new_val)
    return filtered

files = ['sigA.csv', 'sigB.csv', 'sigC.csv', 'sigD.csv']
best_A = {'sigA.csv': 0.999, 'sigB.csv': 0.998, 'sigC.csv': 0.5, 'sigD.csv': 0.97}  

fig, axes = plt.subplots(4, 2, figsize=(12, 16))

for row, fname in enumerate(files):
    t, s = load_csv(fname)
    t = np.array(t)
    sig = np.array(s)

    A = best_A[fname]
    filtered = iir_filter(sig, A)
    filtered = np.array(filtered)

    Fs = len(t) / t[-1]
    frq_raw,  Y_raw  = get_fft(sig,      Fs)
    frq_filt, Y_filt = get_fft(filtered, Fs)

    ax1 = axes[row, 0]
    ax2 = axes[row, 1]

    ax1.plot(t, sig,      'k', linewidth=0.8, label='raw')
    ax1.plot(t, filtered, 'r', linewidth=1.5, label=f'IIR A={A}')
    ax1.set_xlabel('Time')
    ax1.set_ylabel('Amplitude')
    ax1.set_title(f'{fname} — IIR A={A}, B={1-A:.2f}')
    ax1.legend()

    ax2.loglog(frq_raw,  abs(Y_raw),  'k', linewidth=0.8, label='raw FFT')
    ax2.loglog(frq_filt, abs(Y_filt), 'r', linewidth=1.5, label='filtered FFT')
    ax2.set_xlabel('Freq (Hz)')
    ax2.set_ylabel('|Y(freq)|')
    ax2.set_title(f'{fname} — FFT comparison')
    ax2.legend()

plt.tight_layout()
plt.savefig('iir_filtered.png', dpi=150)
plt.show()