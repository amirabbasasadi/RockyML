import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sb
import sys
from io import StringIO

sb.set(style='white')

if(len(sys.argv) < 2):
    raise ValueError("you should pass the path of optimization log data.")

path = sys.argv[1]

try:
    fp = open(path, "r")
    content = fp.read()
    fp.close()
except:
    raise ValueError("log data does not exist")

split_content = content.split('---')
csv_content = split_content[0]
meta_data = split_content[1]
del content
del split_content

csv_data = StringIO(csv_content)
data = pd.read_csv(csv_data)

global_best = data.groupby(['time']).min()['value'].values

plt.title("Best Global Solution")
plt.plot(global_best)
plt.xlabel("iters")
plt.ylabel("value")
plt.savefig("global.png")
plt.close()

n_tribes = data['tribe'].max() + 1
plt.title("Best Tribes Solution")
for t in range(n_tribes):
    tribe = data[data['tribe'] == t]['value'].values
    plt.plot(tribe)

plt.xlabel("iters")
plt.ylabel("value")
plt.savefig("tribes.png")
plt.close()


