from plot import load_csv
from fft import get_fft

import numpy as np
import matplotlib.pyplot as plt

def lowpass_sinc_weights(cutoff_hz, fs, num_taps, window='hamming'):
    fc = cutoff_hz / fs      
    M  = num_taps - 1
    n  = np.arange(num_taps)

    h = np.sinc(2 * fc * (n - M / 2.0))

    windows = {
        'hamming':     np.hamming(num_taps),
        'blackman':    np.blackman(num_taps),
        'hanning':     np.hanning(num_taps),
        'bartlett':    np.bartlett(num_taps),
        'rectangular': np.ones(num_taps),
    }
    h = h * windows.get(window, np.hamming(num_taps))
    h = h / np.sum(h)          
    return h

def apply_fir(signal, weights):
    return np.convolve(signal, weights, mode='same')

params = {
    'sigA.csv': (20,   101, 'hamming'),   
    'sigB.csv': (10,   101, 'hamming'),   
    'sigC.csv': (2,     51, 'hamming'),   
    'sigD.csv': (2,     51, 'blackman'),  
}

files = ['sigA.csv', 'sigB.csv', 'sigC.csv', 'sigD.csv']

fig, axes = plt.subplots(4, 2, figsize=(12, 16))

for row, fname in enumerate(files):
    t, s = load_csv(fname)
    t   = np.array(t)
    sig = np.array(s)

    Fs = len(t) / t[-1]

    cutoff, taps, win = params[fname]
    weights  = lowpass_sinc_weights(cutoff, Fs, taps, window=win)
    filtered = apply_fir(sig, weights)

    frq_raw,  Y_raw  = get_fft(sig,      Fs)
    frq_filt, Y_filt = get_fft(filtered, Fs)

    ax1 = axes[row, 0]
    ax2 = axes[row, 1]

    ax1.plot(t, sig,      'k', linewidth=0.8, label='raw')
    ax1.plot(t, filtered, 'r', linewidth=1.5, label=f'FIR {taps}-tap {win}')
    ax1.set_xlabel('Time')
    ax1.set_ylabel('Amplitude')
    bw = 2 * cutoff
    ax1.set_title(f'{fname} — FIR fc={cutoff}Hz, BW={bw}Hz, {taps}-tap {win}')   
    ax1.legend()

    ax2.loglog(frq_raw,  abs(Y_raw),  'k', linewidth=0.8, label='raw FFT')
    ax2.loglog(frq_filt, abs(Y_filt), 'r', linewidth=1.5, label='filtered FFT')
    ax2.set_xlabel('Freq (Hz)')
    ax2.set_ylabel('|Y(freq)|')
    ax2.set_title(f'{fname} — FFT comparison')
    ax2.legend()

plt.tight_layout()
plt.savefig('fir_filtered.png', dpi=150)
plt.show()