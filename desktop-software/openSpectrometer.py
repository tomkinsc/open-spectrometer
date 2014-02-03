import os, datetime, serial, re, yaml, sys, numpy, time, copy
import _winreg as winreg
import itertools
from collections import OrderedDict 
try:
	import gtk
	from matplotlib.figure import Figure
	from matplotlib.backends.backend_gtkagg import FigureCanvasGTKAgg as FigureCanvas
	from matplotlib.backends.backend_gtkagg import NavigationToolbar2GTKAgg as NavigationToolbar
except ImportError as exception:
	pass
	

	
def index_min(values):
    return min(xrange(len(values)),key=values.__getitem__)
	
def index_max(values):
    return max(xrange(len(values)),key=values.__getitem__)

def enumerate_serial_ports():
	""" Uses the Win32 registry to return an 
	iterator of serial (COM) ports 
	existing on this computer.
	"""
	#need to learn how to tell if it is windows or not, then branch to do appropriate enumeration
	
	path = 'HARDWARE\\DEVICEMAP\\SERIALCOMM'
	try:
		key = winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, path)
	except WindowsError:
		print 'error'
	for i in itertools.count():
		try:
			val = winreg.EnumValue(key, i)
			yield str(val[1])
		except EnvironmentError:
			break
def averageSpectra(listOfSpectra):
	#copy a spectrum so we retain the instrumentName and such in the output
	output=copy.deepcopy(listOfSpectra[0])
	temp=[]
	
	#set all the data in the output to 0
	for wavelength in output['datapoints'].keys():
		temp.append(0)
		output['datapoints'][wavelength]=0
	
	#sum the spectra data
	for spectrum in listOfSpectra:
		for wavelength in output['datapoints'].keys():
			output['datapoints'][wavelength]=output['datapoints'][wavelength]+spectrum['datapoints'][wavelength]
	for wavelength in output['datapoints'].keys():
		output['datapoints'][wavelength]=output['datapoints'][wavelength]/len(listOfSpectra)

	return output
def getLambdaMax(spectrum):
	peakSearchPattern = [1,1,1,-1,-1,-1]

	#this is the list of wavelengths
	xData=numpy.array(spectrum['datapoints'].keys())
	yData=numpy.array(spectrum['datapoints'].values())

	#find peaks on a polyfit
	z=numpy.polyfit(xData, yData,20)
	p = numpy.poly1d(z)
	import matplotlib.pyplot as plt
	import matplotlib
	from matplotlib.widgets import Button
	
	diffData=numpy.diff(p(xData))
	
	i=0
	j=0
	possiblePeaks={}
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
				possiblePeaks[spectrum['datapoints']i]
	yPeaks=[yData[pp] for pp in possiblePeaks]
	xPeaks=possibleWavelengthsOfPeaks
	
	fig1=spectrum_plotter(xData, yData)
	fig1.plotSelectable(xData, yData, 'k.', picker=5)
	fig1.plot(xData, p(xData), 'w-')
	fig1.plot(xPeaks, yPeaks, 'ro', ms=10)
	
	possHiPeak=openSpectrometer.index_max(yPeaks)
	fig1.selectPoint(xPeaks[possHiPeak], yPeaks[possHiPeak])
	fig1.show()

	return possiblePeaks
def calculate_pH(experiment_config_file_path):
	cfgFile=yaml.load(open(experiment_config_file_path), OrderedDictYAMLLoader)
 
    assert type(cfgFile) is OrderedDict
	
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
		try:
			acidData.append(yaml.load(open(fileName)))
		except IOError:
			f=os.path.join(os.path.dirname(experiment_config_file_path),fileName)
			print '*****'
			print f
			print '*****'
			acidData.append(yaml.load(open(f)))
	for fileName in cfgFile['baseFileList'].split(','):
		try:
			baseData.append(yaml.load(open(fileName)))
		except IOError:
			baseData.append(yaml.load(open(os.path.join(os.path.dirname(experiment_config_file_path),fileName))))
	for fileName in cfgFile['bufferFileList'].split(','):
		try:
			bufferData.append(yaml.load(open(fileName)))
		except IOError:
			bufferData.append(yaml.load(open(os.path.join(os.path.dirname(experiment_config_file_path),fileName))))	
	return getLambdaMax(averageSpectra(acidData))
class spectrum_plotter:
	def __init__(self, xs, ys):
		self.xs=xs
		self.ys=ys
		
		self.win = gtk.Window()
		self.win.connect("destroy", lambda x: gtk.main_quit())
		self.win.set_default_size(800,600)
		self.win.set_title("openSpectrometer")

		self.vbox = gtk.VBox()
		self.win.add(self.vbox)
		
		self.fig = Figure(figsize=(5,4), dpi=100)
		self.canvas = FigureCanvas(self.fig)  # a gtk.DrawingArea
		
		self.ax = self.fig.add_subplot(111)
		
		self.canvas.mpl_connect('pick_event', self.onpick)
		
		self.vbox.pack_start(self.canvas)
		self.toolbar = NavigationToolbar(self.canvas, self.win)
		self.hbox = gtk.HBox()
		self.button=gtk.Button('Select this point as lambda max')
		self.button.connect("clicked", self.buttonClick)
		self.hbox.pack_start(self.toolbar)
		self.hbox.pack_start(self.button)
		self.vbox.pack_start(self.hbox, False, False)
		
		self.lastind = 0

		self.text = self.ax.text(0.05, 0.95, 'Datapoint index selected: none',
                            transform=self.ax.transAxes, va='top')
	def plot(self, *args, **kwargs):
		self.ax.plot(*args, **kwargs)
	def plotSelectable(self, *args, **kwargs):
		self.line=self.ax.plot(*args, **kwargs)
	def selectPoint(self, x, y):
		self.selected = self.ax.plot([x],[y], 'o', ms=20, alpha=0.4, 
									   color='yellow', visible=False)
	def show(self):
		self.win.show_all()
		gtk.main()
	def buttonClick(self):
		return self.lastind
	def onpick(self, event):
		if event.artist!=self.line[0]: return True
		
		N = len(event.ind)
		if not N: return True
		print 'here'

		if N > 1:
			print '%i points found!' % N
		print event.ind

		# the click locations
		x = event.mouseevent.xdata
		y = event.mouseevent.ydata

		dx = numpy.array(x-self.xs[event.ind],dtype=float)
		dy = numpy.array(y-self.ys[event.ind],dtype=float)

		distances = numpy.hypot(dx,dy)
		indmin = distances.argmin()
		dataind = event.ind[indmin]

		self.lastind = dataind
		self.update()

	def update(self):
		if self.lastind is None: return

		dataind = self.lastind
		
		self.selected[0].set_visible(True)
		self.selected[0].set_data(self.xs[dataind], self.ys[dataind])

		# put a user function in here!        
		self.userfunc(dataind)

		self.fig.canvas.draw()

	def userfunc(self,dataind):
		self.text.set_text('datapoint index selected: %d'%dataind)
		print 'No userfunc defined'
		pass
		
class Spectrometer(object):
	def __init__(self):
		raise NotImplementedError("This must be implemented by the subclass.")
	def connect(self):
		raise NotImplementedError("This must be implemented by the subclass.")
	def disconnect(self):
		raise NotImplementedError("This must be implemented by the subclass.")
	def detect(self):
		raise NotImplementedError("This must be implemented by the subclass.")
	def read(self):
		raise NotImplementedError("This must be implemented by the subclass.")
	def write(self):
		raise NotImplementedError("This must be implemented by the subclass.")
	
class openSpectrometer(spectrometer):
	def __init__(self):
		self.instrumentName='openSpectrometer'
		raise NotImplementedError("This section isn't developed yet.")
	def connect(self):
		raise NotImplementedError("This section isn't developed yet.")
	def disconnect(self):
		raise NotImplementedError("This section isn't developed yet.")
	def detect(self):
		raise NotImplementedError("This section isn't developed yet.")
	def read(self):
		raise NotImplementedError("This section isn't developed yet.")
	def write(self):
		raise NotImplementedError("This section isn't developed yet.")
	
		
class Biowave_II(spectrometer):
	def __init__():
		self.instrumentName='Biowave II'
	def connect(self, port=None):
		if port==None:
			for i in enumerate_serial_ports():
				self.ser=serial.Serial(port=i, baudrate=115200, rtscts=True, dsrdtr=True, timeout=5)
				if self.detect():
					return True
		else:
			self.ser=serial.Serial(port=port, baudrate=115200, rtscts=True, dsrdtr=True, timeout=5)	
	def disconnect(self):
		self.ser.close()
		self.ser=None
	def detect():
		print 'Press the \'Take Reading\' button on the spectrometer\nWaiting 5 seconds'
		end_time=time.time()+5
		while(time.time()<end_time):
			if self.instrumentName in self.ser.readline():
				return True
		return False
	def read(self):
		print 'Starting to look for data'
		startWavelength=None; endWavelength=None; datapoints=[]; datapointsReady=False; lastLine=''
		while(1):
			line=self.ser.readline()
			print 'got a line of data: ' + line
			if "$PAX" in line:
				startWavelength, endWavelength = [int(x) for x in line.split()[4:6]]
				minAbs, maxAbs = [float(x) for x in line.split()[9:11]]
				lastLine=line
			elif "$PPY" in line and len(line.split())>7:
				datapoints= ' '.join(line.split()[5:])
				lastLine=line
			elif "$PPY" in line and len(line.split())==7:
				datapointsReady=True
				datapoints=re.sub('[\n\r]', '', datapoints)
				datapoints=re.sub(r'([0-9])0\.([0-9])', r'\1 0.\2', datapoints)
				datapoints=[float(x) for x in datapoints.split()]
				lastLine=''
			elif "$PPY" in lastLine:
				datapoints+=line
			if datapointsReady==True:
				datapoints =  OrderedDict([(wavelengths[index], ad_voltages[index]) for index in 
										range(0, min([len(ad_voltages), len(wavelengths)]))])
				return OrderedDict([("openSpectrometer data file" , "version 0"), ("sample ID", sampleID),	("startWavelength" , startWavelength), 
									("endWavelength" , endWavelength), ("minAbsorbanceDataValue" , minAbs), 
									("maxAbsorbanceDataValue" , maxAbs), ("datapoints" , datapoints)])
	


if __name__ == '__main__':
	if len(sys.argv)>1:
		spectrometer=Biowave_II()
		fileName = 'feExperiment'+datetime.datetime.now().strftime("%d_%m_%y__%H_%M_%S")
		while(1):
			try:
				sampleID=""
				while (enterSampleIds==True and sampleID==""):
					print 'enter sample ID'
					sampleID = raw_input().strip()
					spectrum = spectrometer.read()
				f=open(fileName,'a')
				f.write(yaml.dump(spectrum))
				f.close()
			except KeyboardInterrupt:
				print 'Done taking samples'
		spectrometer.close()