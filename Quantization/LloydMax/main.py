# ==== Lloyd-Max for Gaussian, Uniform and Laplacian distribution
import numpy as np
from LloydMax import LloydMaxQuant

# --- Initialization
source = 'laplace'
if source == 'gaussian':
    parameter = 1 # Variance
elif source == 'uniform':
    parameter = np.sqrt(12)/2.0 # b: maximum value of the uniform
elif source == 'laplace':
    parameter = 1.0/np.sqrt(2) # Laplacian parameter

# Rate
R = 5
# Number of cells
nCells = 2 ** R
# Initial Centroids
initialCentroids = np.random.uniform(low=-3*parameter, high=3*parameter, size=nCells)

# =========== Example: Gaussian Source =========== #

# --- Create a Quantizer
quantizer = LloydMaxQuant(initialCentroids, parameter, source)

# --- Generate at random a value to quantize
x = np.random.normal(loc=0, scale=1,size=1)

# --- Quantize a random value
y = quantizer.Q(x)

# --- From index to reconstruction value
xHat = quantizer.Qinv(y)

print('Value to quantize: ' + str(np.squeeze(x)))
print('Index of the cell: ' + str(y))
print('Reconstruction value: ' + str(xHat))




