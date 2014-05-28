import sys
import os
import numpy as np
from mpl_toolkits.mplot3d import Axes3D
import matplotlib
matplotlib.use ( "Agg" )
import matplotlib.pyplot as plt
import matplotlib.animation as anim

fname = sys.argv[1]
n = 0

#data = np.genfromtxt(fname, dtype=[('string','S8'),('int','i8'),('string2','S8')], delimiter=",")
data = np.genfromtxt(fname, dtype=[('string','S8'),('cost','i4'),('string2','S8'),('matches','int_')], delimiter=",", deletechars=";", usecols=[1,3])
#data = np.genfromtxt(fname, dtype=None, delimiter=",")
print data

n = len(data)

X = data['cost']
Y = data['matches']

plt.ion ()

fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')

hist, xedges, yedges = np.histogram2d(X, Y, bins=20)

elements = (len(xedges) - 1) * (len(yedges) - 1)
xpos, ypos = np.meshgrid(xedges[:-1]+0.25, yedges[:-1]+0.25)

xpos = xpos.flatten()
ypos = ypos.flatten()
zpos = np.zeros(elements)
dx = 0.5 * np.ones_like(zpos)
dy = dx.copy()
dz = hist.flatten()

ax.set_xlabel('Custo')
ax.set_ylabel('Incidencias')
ax.set_zlabel('Z')

ax.bar3d(xpos, ypos, zpos, dx, dy, dz, color='b', zsort='average')
#ax.plot_surface(range(100), ypos[:100], xpos[:100], color='b', rstride=4, cstride=4)
ax.legend()

"""
for angle in range(0, 360):
            ax.view_init(30, angle)
            plt.draw()
"""

FFMpegWriter = anim.writers['ffmpeg']
metadata = dict(title='Movie Test', artist='Matplotlib',
                comment='Movie support!')
writer = FFMpegWriter(fps=15, metadata=metadata)

with writer.saving(fig, "writer_test.mp4", 100):
    for angle in range(0, 360):
                ax.view_init(30, angle)
    #            plt.draw()
                writer.grab_frame()


#plt.show()

#plt.plot(data)
#plt.plot ( [ 0, 1,451 ], [ 5, 1, 1 ], 'ro' )


#plt.axis([0, 6, 0, 20])

"""
from mpl_toolkits.mplot3d import Axes3D
import numpy as np
import matplotlib.pyplot as plt

fig = plt.figure()
ax = fig.gca(projection='3d')

x = np.linspace(0, 1, 100)
y = np.sin(x * 2 * np.pi) / 2 + 0.5
ax.plot(x, y, zs=0, zdir='z', label='zs=0, zdir=z')

colors = ('r', 'g', 'b', 'k')
X2 = X[0:5]
print X2
for c, x in (colors, X2):
    #x = np.random.sample(20)
    y = np.random.sample(20)
    ax.scatter(x, y, 0, zdir='y', c=c)

ax.legend()
ax.set_xlim3d(0, 1)
ax.set_ylim3d(0, 1)
ax.set_zlim3d(0, 1)

plt.show()
"""
