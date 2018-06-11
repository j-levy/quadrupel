import matplotlib.pyplot as plt
import matplotlib.animation as animation
import time
import numpy as np

fig = plt.figure()
ax1 = fig.add_subplot(1,1,1)
af = [0,0,0]
bf = [0,0,0]
"""
y[n] = (  1 * x[n- 2])
     + (  2 * x[n- 1])
     + (  1 * x[n- 0])

     + ( -0.9149758348 * y[n- 2])
     + (  1.9111970674 * y[n- 1])
"""
gain = 1.058546241e+03
af[0] = 1 / gain
af[1] = 2 / gain
af[2] = 1 / gain
bf[0] = 0
bf[1] = 1.9111970674
bf[2] = -0.9149758348

xf = [0,0,0]
yf = [0,0,0]

def animate(i):
    pullData = open("filtered_data.csv","r").read()

    dataArray = pullData.split('\n')
    xar = []
    yar = []
    y2ar = []
    yglod = []
    for eachLine in dataArray:
        if len(eachLine)>1:
            x,y,y2 = eachLine.split(',')
            xf[2] = xf[1]
            xf[1] = xf[0]
            xf[0] = int(y)
            yf[2] = yf[1]
            yf[1] = yf[0]
            yf[0] = xf[0]*af[0] + xf[1]*af[1] + xf[2]*af[2] + yf[1]*bf[1] + yf[2]*bf[2]
            xar.append(int(x))
            yar.append(int(y))
            yglod.append(yf[0])
            y2ar.append(int(y2))

    ax1.clear()
    ax1.plot(xar,yar, color = 'b')
    ax1.plot(xar, y2ar , color = 'r')
    ax1.plot(xar, yglod , color = 'g')

ani = animation.FuncAnimation(fig, animate, interval=5)
plt.show()
