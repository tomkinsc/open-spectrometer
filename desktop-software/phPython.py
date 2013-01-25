import sys
import json
import numpy as np
import copy 

currentLambdaMax=0

def averageSpectra(jsonSpectra):
	output=copy.deepcopy(jsonSpectra[0])
	output['datapoints']=[0]

	for spectrum in jsonSpectra:
		print len(spectrum['datapoints'])
		output['datapoints']=np.add(output['datapoints'], spectrum['datapoints'])
	
	output['datapoints']=np.divide(output['datapoints'], len(jsonSpectra))
	print len(output['datapoints'])
	return output


		
def getLambdaMax(jsonSpectrum):
	peakSearchPattern = [1,1,1,-1,-1,-1]

	
	#this is the list of wavelengths
	xData=np.array(range(jsonSpectrum['startWavelength'], jsonSpectrum['endWavelength']+1))
	yData=np.array(jsonSpectrum['datapoints'])

	#find peaks on a polyfit
	z=np.polyfit(xData, yData,20)
	p = np.poly1d(z)
	import matplotlib.pyplot as plt
	import matplotlib
	from matplotlib.widgets import Button
	
	diffData=np.diff(p(xData))
	
	i=0
	j=0
	possiblePeaks=[]
	for i in range(0,len(diffData)-1):
		if i>=((len(peakSearchPattern)/2)-1) and i<=(len(diffData)-(len(peakSearchPattern)/2)-1):
			j=i-(len(peakSearchPattern)/2)
			counter=0
			for element in peakSearchPattern:
				if element==1 and diffData[j]<0:
					break
				if element==-1 and diffData[j]>0:
					break
				counter+=1
				j+=1
			if counter==len(peakSearchPattern):
				possiblePeaks.append(i)
	possibleWavelengthsOfPeaks=np.add(possiblePeaks, jsonSpectrum['startWavelength'])
	yPeaks=[yData[pp] for pp in possiblePeaks]
	xPeaks=possibleWavelengthsOfPeaks
	
	import openSpectrometer
	fig1=openSpectrometer.plotter(xData, yData)
	fig1.plotSelectable(xData, yData, 'k.', picker=5)
	fig1.plot(xData, p(xData), 'w-')
	fig1.plot(xPeaks, yPeaks, 'ro', ms=10)
	
	possHiPeak=openSpectrometer.index_max(yPeaks)
	fig1.selectPoint(xPeaks[possHiPeak], yPeaks[possHiPeak])
	fig1.show()
	class buttonHandler:
		def ok(self, event):
			plt.close()
			
	#butt=buttonHandler()
	#okButton = Button(plt.axes([0.81, 0.0, 0.1, 0.01]), 'OK')
	
	#okButton.on_clicked(butt.ok)
	#plt.show()
	return possiblePeaks

if __name__ == '__main__':
	if len(sys.argv)!=2:
		print 'the only argument should be config file path'
		quit()
	cfgFile=json.load(open(sys.argv[1]))
	
	if cfgFile['openSpectrometer experiment name']!='pH determination':
		print 'config file not for pH experiment'
		quit()

	
	baseLambdaMax=0
	acidLambdaMax=0
	
	baseAvgValueAtBaseLambdaMax=0
	acidAvgValueAtAcidLambdaMax=0
	
	bufferAvgValueAtBaseLambdaMax=0
	bufferAvgValueAtAcidLambdaMax=0
	
	baseData=[]
	acidData=[]
	bufferData=[]

	for fileName in cfgFile['acidFileList'].split(','):
		acidData.append(json.load(open(fileName)))
	for fileName in cfgFile['baseFileList'].split(','):
		baseData.append(json.load(open(fileName)))
	for fileName in cfgFile['bufferFileList'].split(','):
		bufferData.append(json.load(open(fileName)))
	
	
	getLambdaMax(averageSpectra(acidData))