from plot import load_csv

import numpy as np
import matplotlib.pyplot as plt

def get_fft(signal, Fs):
    """Returns (frequencies, complex FFT values) for a signal."""
    y = np.array(signal)
    n = len(y)
    k = np.arange(n)
    T = n / Fs
    frq = k / T
    frq = frq[range(int(n/2))]
    Y = np.fft.fft(y) / n
    Y = Y[range(int(n/2))]
    return frq, Y

if __name__ == '__main__':
    files = ['sigA.csv', 'sigB.csv', 'sigC.csv', 'sigD.csv']

    fig, axes = plt.subplots(4, 2, figsize=(12, 16))

    for row, fname in enumerate(files):
        t, s = load_csv(fname)
        t = np.array(t)
        s = np.array(s)

        Fs = len(t) / t[-1]
        frq, Y = get_fft(s, Fs)

        ax1 = axes[row, 0]
        ax2 = axes[row, 1]

        ax1.plot(t, s, 'b')
        ax1.set_xlabel('Time')
        ax1.set_ylabel('Amplitude')
        ax1.set_title(f'{fname} — time domain')

        ax2.loglog(frq, abs(Y), 'b')
        ax2.set_xlabel('Freq (Hz)')
        ax2.set_ylabel('|Y(freq)|')
        ax2.set_title(f'{fname} — FFT')

    plt.tight_layout()
    plt.savefig('all_ffts.png', dpi=150)
    plt.show()