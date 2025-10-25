#!/usr/bin/env python3
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

color_blue   = "#2056ae"
color_yellow = "#fab20b"
color_grey   = "#858270"


def main():
    print("At first, measure hits and misses with\n" +
          "  ./exercise_plot > measurement.csv")
    plot_ab("measurement", "cpu cycles", "count")

def plot_ab(name, xlabel, ylabel):
  
  # read data
  csv = pd.read_csv(f"{name}.csv")
  cached   = np.asarray(csv["cached"], dtype=np.int32)
  uncached = np.asarray(csv["uncached"], dtype=np.int32)
  minimum = min(min(cached), min(uncached))
  maximum = max(max(cached), max(uncached))
  maximum = min(maximum, 500)
  assert len(cached) == len(uncached)
  print("Min:{}\nMax:{}".format(minimum, maximum))

  fig, ax = plt.subplots(1,1,figsize=(10,5))
  ax.set_xlabel(xlabel)
  ax.set_ylabel(ylabel)

  bins = np.arange(minimum, maximum + 1, 20)

  ax.hist(cached,   bins, alpha=0.7, label="Cached", color=color_blue)
  ax.hist(uncached, bins, alpha=0.7, label="Uncached", color=color_yellow)
  
  fig.savefig(f"{name}.pdf")
  fig.savefig(f"{name}.png")

if __name__ == "__main__":
    main()
