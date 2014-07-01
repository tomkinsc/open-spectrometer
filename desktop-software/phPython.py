import sys
import json
import numpy as np
import copy 
from collections import OrderedDict 
currentLambdaMax=0
import openSpectrometer

if __name__ == '__main__':
	if len(sys.argv)!=2:
		print 'the only argument should be config file path'
		quit()
	openSpectrometer.calculate_pH(sys.argv[1])