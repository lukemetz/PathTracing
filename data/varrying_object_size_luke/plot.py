import glob
from matplotlib import pyplot
import sys


def parse_file(file):
	print file
	diction = {}
	data = open(file)
	for f in data:
		k = f.split("=")
		if len(k) > 1 and k[1] != ' ':
			diction[k[0]] = float(k[1].strip())
	return diction

files = glob.glob("*.txt")

sphere_num = []
time_between_frames = []
for f in files:
	data = parse_file(f)
	sphere_num.append(int(f.split('.')[0]))
	time_between_frames.append(data['avg_time_diff'])

pyplot.plot(sphere_num, time_between_frames, 'o', alpha=.5)
pyplot.ylabel("Time per frame (sec)")
pyplot.xlabel("Number of Spheres")
pyplot.title("Performance Based on Work Group Size")
pyplot.legend(["NV 4200M on small image", "Alecs computer here"])
pyplot.show()

