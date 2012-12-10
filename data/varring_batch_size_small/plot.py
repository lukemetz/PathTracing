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

work_groups = []
time_between_frames = []
for f in files:
	data = parse_file(f)
	work_groups.append(data['worksize'])
	time_between_frames.append(data['avg_time_diff'])

pyplot.loglog(work_groups, time_between_frames, 'o')
pyplot.ylabel("Time per frame (sec)")
pyplot.xlabel("Work group Size")
pyplot.title("Performance Based on Work Group Size")
pyplot.legend(["NV 4200M on small image", "Alecs computer here"])
pyplot.show()

