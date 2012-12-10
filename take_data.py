import os
import numpy
datapoints = [x*10000 for x in range(1,50)]
#datapoints.reverse()

for data in datapoints:
	os.system("./cldemo "+str(data)+" "+str(data)+".txt")
