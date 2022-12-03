import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sb
import sys

if(len(sys.argv) < 2):
    raise ValueError("you should pass the path of optimization log data.")

path = sys.argv[1]

try:
    data = pd.read_csv(path)
except:
    raise ValueError("log data does not exist")

global_best = data.groupby(['time']).min()['value'].values

plt.title("Best Global Solution")
plt.plot(global_best)
plt.xlabel("iters")
plt.ylabel("value")
plt.show()

n_tribes = data['tribe'].max() + 1
plt.title("Best Tribes Solution")
for t in range(n_tribes):
    tribe = data[data['tribe'] == t]['value'].values
    plt.plot(tribe, label=f'tribe {t}')

plt.legend()
plt.xlabel("iters")
plt.ylabel("value")
plt.show()


