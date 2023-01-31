#    Copyright (C) 2022 Amirabbas Asadi , All Rights Reserved
#    distributed under Apache-2.0 license

import click

import numpy as np
import matplotlib.pyplot as plt
import os

def get_output_path(path):
  head, tail = os.path.split(path)
  ds = tail.split('.')
  fname = '.'.join(ds[:-1])
  output_path = os.path.join(head, '{}.png'.format(fname))
  return output_path

def read_zagros_mesh(path):
  data = {}
  fp = open(path)
  content = fp.read().split('\n')
  data['step'] = content[0]
  wh = content[2].split(" ")
  data['height'] = int(wh[0])
  data['width'] = int(wh[1])
  space = content[1].split(" ")
  data['x_min'] = float(space[0])
  data['y_min'] = float(space[1])
  data['x_max'] = float(space[2])
  data['y_max'] = float(space[3])
  data['z'] = np.fromstring(content[3], sep=' ').reshape((data['height'],
                                                          data['width']))
  return data

def plot(in_path, contour=True, dpi=96):
    data = read_zagros_mesh(in_path)
    output_path = get_output_path(in_path)

    delta_x = (data['x_max'] - data['x_min']) / data['width']
    delta_y = (data['y_max'] - data['y_min']) / data['height']
    x = np.arange(data['x_min'], data['x_max'], delta_x)
    y = np.arange(data['y_min'], data['y_max'], delta_y)
    X, Y = np.meshgrid(x, y)

    plt.figure(figsize=(data['height']/65, data['width']/65))
    plt.imshow(data['z'], cmap='plasma_r')
    if contour:
      X /= delta_x
      Y /= delta_y
      plt.contour(X + X.max() + 1, Y + Y.max() +1, data['z'], colors='k',
                  levels=5, linewidths=2.0)
    plt.axis('off')
    plt.savefig(output_path, bbox_inches='tight', transparent="True", pad_inches=0)


@click.command()
@click.option('--files', help='input files')
def generate(files):
  for file_path in files.split(' '):
    print('plotting {} ...'.format(file_path))
    plot(file_path)

if __name__ == '__main__':
    generate()
