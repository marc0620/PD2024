import numpy as np
import matplotlib.pyplot as plt

def plot_array(filename):
  # Read the array from file
  array = np.loadtxt(filename)
  n = array.shape[0]
  print(n)
  # Plot the array as a colored grid
  plt.imshow(array, cmap='viridis', interpolation='nearest')
  plt.colorbar(label='Value')
  plt.title('Visualization of Array')
  plt.xticks(np.arange(n))
  plt.yticks(np.arange(n))
  plt.grid(True)
  plt.show()

# Example usage
filename = './tempout.txt'  # Specify the filename containing the array
plot_array(filename)