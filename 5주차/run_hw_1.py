#!/usr/bin/python3
import sys, os
from shutil import copyfile

try :
    case = sys.argv[1]
except :
    case = "0"

directory = "/home/kwonl/tools/ns-allinone-3.29/ns-3.29"
files = os.listdir(directory)
for file in files:
    if file.endswith(".pcap"):
        os.remove(os.path.join(directory,file))

if case == "0" :
    os.system("./waf --run scratch/first")
elif case == "1" :
    # compare csma p2p
    os.system("./waf --run 'scratch/first -csma=0'")
elif case == "2" :
    os.system("./waf --run 'scratch/first -node4DataRate=0.5Mbps'")
elif case == "3" :
    os.system("./waf --run 'scratch/first -node4DataRate=1Mbps'")
elif case == "4" :
    os.system("./waf --run 'scratch/first -node4DataRate=2Mbps'")
else :
    print("invalid input")