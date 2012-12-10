import glob
import matplotlib.pyplot as plt
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
	time_between_frames.append(data['FPS'])

plt.plot(work_groups, time_between_frames,'o')
plt.show()

