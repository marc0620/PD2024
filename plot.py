import matplotlib.pyplot as plt
import numpy

iteration=300000
lamb = 0.99995
init_temp= 1000
arr=numpy.zeros(iteration)
arr[0]=init_temp
dot=False
for i in range(1,iteration):
  arr[i]=arr[i-1]*lamb
  if arr[i]<1 and not dot:
    plt.plot(i,arr[i],'ro')
    dot=True

plt.plot(arr)

plt.show()

