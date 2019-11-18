from random import random
from random import randint
from random import seed
seed(2)

f = open("validationData.dat","w+")

def newQ(size):
	q = []
	for j in range(size):
		r = randint(0,10)
		if r > 6 and len(q) > 0:
			q.append(round(q[len(q)-1]+random(),1))
		else:
			q.append(round(random()*300,1))
	q.sort()
	return q

for i in range(1,200):
	data = newQ(10)
	f.write("%d, 10," % i)
	for point in data:
		f.write(" %.1f," % point)
	f.write("\n")
f.close()
