import pandas as pd

import matplotlib.pyplot as plt

# Read the CSV file
df = pd.read_csv('tmp/results.csv')

# Plotting
plt.figure(figsize=(8, 6))
plt.plot(df['Count'], df['Serial'], label='Serial', marker='o')
plt.plot(df['Count'], df['Parallel'], label='Parallel', marker='o')
plt.xlabel('Count')
plt.ylabel('Time')
plt.title('Serial vs Parallel Execution Time')
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig('tmp/results.png')
