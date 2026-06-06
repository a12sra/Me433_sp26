import csv
import matplotlib.pyplot as plt

def load_csv(filename):
    times, values = [], []
    with open(filename) as f:
        for row in csv.reader(f):
            times.append(float(row[0]))
            values.append(float(row[1]))
    return times, values

files = ['sigA.csv', 'sigB.csv', 'sigC.csv', 'sigD.csv']

fig, axes = plt.subplots(2, 2, figsize=(12, 8))
axes = axes.flatten()

for i, fname in enumerate(files):
    t, sig = load_csv(fname)
    fs = len(t) / t[-1]
    axes[i].plot(t, sig, 'k', linewidth=0.8)
    axes[i].set_xlabel('Time (s)')
    axes[i].set_ylabel('Amplitude')
    axes[i].set_title(f'{fname} — Fs={fs:.0f} Hz')
    axes[i].grid(True, alpha=0.3)

plt.tight_layout()
plt.savefig('signals_raw.png', dpi=150)
plt.show()