#!/usr/bin/python
import sys, os

for nWifi in ['3', '5', '9'] :
    for thre in ['0', '2000'] :
        for protocol in ['0', '1'] :
            os.system('./waf --run "scratch/third ' + '-nWifi=' + nWifi + ' -thre=' + thre + ' -protocol=' + protocol + '" > res_nWifi_' + nWifi +'_thre_' + thre +'_protocol_' + protocol + '.dat 2>&1')
