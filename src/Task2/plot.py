import numpy as np
import matplotlib.pyplot as plt
from scipy.interpolate import griddata

with open('../../cmake-build-release/task2_results.csv') as file:
    lines = file.readlines()

points = []
for line in lines:
    points.append(list(map(float, line.strip().split(';'))))

points = np.array(points, dtype=float)
points_black = np.array(list(filter(lambda x: abs(x[2]) <= 1e-6, points)))
x = points_black[:, 0]
y = points_black[:, 1]
z = points_black[:, 2]

# grid_x, grid_y = np.meshgrid(x, y)
# grid_z = griddata((x, y), z, (grid_x, grid_y), method='cubic')
#
# plt.contourf(grid_x, grid_y, grid_z, cmap='viridis')
# plt.show()
#
plt.plot(x, y, marker='o', markersize=1, linestyle='')
plt.show()
