#!/usr/bin/python

import subprocess

# print command
# subprocess.call(["ls", "-l"])
w_values = [0.2, 0.4, 0.6, 0.8, 1.0]
# w_values = [0.2, 0.4]
for w in w_values:
    # print 'hi'
    command = './waf "--run=scratch/aodv/aodv --w=%f"' % w
    subprocess.call(command, shell=True)
# subprocess.call("exit 1", shell=True)

