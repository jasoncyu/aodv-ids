#!/usr/bin/python

import subprocess
import math
import argparse


parser = argparse.ArgumentParser(description='Run AODV Simulation')
parser.add_argument('--time', type=float, help="Total time of simulation", default=50.0)
parser.add_argument('--w_max', type=float, help="Maximum w we'll try", default=10.0)
parser.add_argument('--w_step', type=float, help="step_size between w's that we'll use", default=2.0)
parser.add_argument('--tau', type=float, help="Threshold", default=0.2)

args = parser.parse_args()
number = math.floor(args.w_max / args.w_step)
number = int(number)
w_values = [x * args.w_step for x in range(1, number + 1)]

f = open("RESULT", 'w')
f.write("Running %d simulations..." % (number))
f.close()

for w in w_values:
    command = "./waf "
    command += '"--run=scratch/aodv/aodv '
    command += '--w=%f' % w
    command += '--time=%f' % args.time
    command += '--tau=%f' % args.tau

    command += '"'
    print command
    subprocess.call(command, shell=True)

subprocess.call("exit 1", shell=True)
