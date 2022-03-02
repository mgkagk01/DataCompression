import numpy as np
from scipy import special as sp
from scipy.integrate import quad
class LloydMaxQuant():
    def __init__(self, initialCentroids, parameter, source='gaussian', epsilon=1e-9):
        self.y = initialCentroids
        self.epsilon = epsilon
        self.nCells = initialCentroids.shape[0]
        self.nBoundary = self.nCells - 1
        self.parameter = parameter
        # Boundary Points
        self.x = np.zeros(self.nBoundary)
        self.source = source
        self.higherValue = np.inf
        self.lowerValue = -np.inf
        if self.source == 'uniform':
            self.higherValue = parameter
            self.lowerValue = -parameter
        createQuantizer(self)

    def Q(self,x):
        distance = (x-self.y)**2
        return int(np.argmin(distance))

    def Qinv(self,index):
        return self.y[index]

def createQuantizer(self):

    temp = np.inf
    while True:
        # --- Step 1: Compute the Boundary Points
        j = 0
        for i in range(self.nBoundary):
            self.x[i] = np.sum(self.y[i:i+2]) / 2.0


        # --- Step 2: Compute the Centroids
        j = 0
        for i in range(self.nCells):
            if i == 0:
                # --- Use lower value
                self.y[i] = computeCentroids(self.lowerValue, self.x[j], self.parameter, self.source,)
            elif i == self.nCells-1:
                # --- Use higher bound
                self.y[i] = computeCentroids(self.x[j], self.higherValue, self.parameter, self.source)
            else:
                self.y[i] = computeCentroids(self.x[j], self.x[j+1], self.parameter, self.source)
                j += 1

        # --- Compute the difference between the new centroids and the previous centroids
        diff = (np.sum(self.y - temp) ** 2) / self.nCells
        d = computeDistortion(self)

        if abs(d - temp)  < self.epsilon:
            break
        else:
            temp = d

    print('Reconstruction Values')
    print(self.y)
    print('SNR(dB) = ' + str(10*np.log10(1/d)))



def computeCentroids(a,b,c,type,x=0):
    if type == 'gaussian':
        # --- Enumerator
        enu = - c * (np.exp(-(b**2)/(c * 2.0)) - np.exp(-(a**2)/(c * 2.0)))
        # --- Denominator
        constant = np.sqrt(c * 2 * np.pi) / 2.0
        den = constant * sp.erf(b / np.sqrt(2.0 * c)) - constant * sp.erf(a / np.sqrt(2.0 * c))

    elif type == 'uniform':
        # --- Enumerator
        enu = (1/(4*c)) * (b**2 - a**2)
        # --- Denominator
        den = (1/(2*c)) * (b - a)

    elif type == 'laplace':
        enu, _ = quad(integralLEnu, a, b, args=(c))
        den, _ = quad(integralLDen, a, b, args=(c))


    return enu/den

def computeDistortion(self):

    j = 0
    c = 2.0 * self.parameter
    result = 0
    for i in range(self.nCells):

        d = self.y[i]
        if i == 0:
            # --- Use lower value
            if self.source == 'gaussian':
                I,_ = quad(integralG, self.lowerValue, self.x[j], args=(c,d)) / np.sqrt(2*np.pi*self.parameter)
            elif self.source == 'uniform':
                I = (1.0 / (3 * 2 * self.parameter)) * ((self.x[j] - self.y[i])**3 - (self.lowerValue - self.y[i])**3)
            elif self.source == 'laplace':
                I,_ = quad(integralL, self.lowerValue, self.x[j], args=(self.parameter, self.y[i]))
        elif i == self.nCells - 1:
            # --- Use higher bound
            if self.source == 'gaussian':
                I,_ = quad(integralG, self.x[j], self.higherValue, args=(c,d)) / np.sqrt(2*np.pi*self.parameter)
            elif self.source == 'uniform':
                I = (1.0/(3*2*self.parameter)) * ((self.higherValue - self.y[i])**3 - (self.x[j] - self.y[i])**3)
            elif self.source == 'laplace':
                I, _ = quad(integralL, self.x[j], self.higherValue, args=(self.parameter, self.y[i]))

        else:
            if self.source == 'gaussian':
                I,_ = quad(integralG, self.x[j], self.x[j+1], args=(c, d)) / np.sqrt(2*np.pi*self.parameter)
            elif self.source == 'uniform':
                I = (1.0/(3*2*self.parameter)) * ((self.x[j+1] - self.y[i])**3 - (self.x[j] - self.y[i])**3)
            elif self.source == 'laplace':
                I,_ = quad(integralL, self.x[j], self.x[j+1], args=(self.parameter, self.y[i]))
            j += 1
        result += I

    return result

def integralLDen(x,c):
    return (1/(2*c))*np.exp(-abs(x)/c)

def integralLEnu(x,c):
    return x*(1/(2*c))*np.exp(-abs(x)/c)

def integralL(x,c,d):
    return (1 / (2 * c)) * ((x-d)**2) * np.exp(-abs(x) / c)

def integralG(x,c,d):
    return ((x-d)**2) * np.exp(-(x**2)/c)


