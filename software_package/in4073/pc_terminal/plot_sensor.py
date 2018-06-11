import matplotlib.pyplot as plt
import matplotlib.animation as animation
import time

fig = plt.figure()
ax1 = fig.add_subplot(1,1,1)

def animate(i):
    pullData = open("filtered_data.csv","r").read()
    dataArray = pullData.split('\n')
    xar = []
    yar = []
    y2ar = []
    for eachLine in dataArray:
        if len(eachLine)>1:
            x,y,y2 = eachLine.split(',')
            xar.append(int(x))
            yar.append(int(y))
            y2ar.append(int(y2))
    ax1.clear()
    ax1.plot(xar,yar, color = 'b')
    ax1.plot(xar, y2ar , color = 'r')

ani = animation.FuncAnimation(fig, animate, interval=20)
plt.show()
