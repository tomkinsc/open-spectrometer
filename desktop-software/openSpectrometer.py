from matplotlib.figure import Figure
from matplotlib.backends.backend_gtkagg import FigureCanvasGTKAgg as FigureCanvas
from matplotlib.backends.backend_gtkagg import NavigationToolbar2GTKAgg as NavigationToolbar
import os, datetime, serial, re, json, sys, numpy, gtk

def index_min(values):
    return min(xrange(len(values)),key=values.__getitem__)
	
def index_max(values):
    return max(xrange(len(values)),key=values.__getitem__)

class spectrometer:
	def __init__(self):
		raise NotImplementedError("This must be implemented by the subclass.")
	def read(self):
		raise NotImplementedError("This must be implemented by the subclass.")
	def write(self):
		raise NotImplementedError("This must be implemented by the subclass.")
	def detect(self):
		raise NotImplementedError("This must be implemented by the subclass.")
	
class openSpectrometer:
	def __init__(self, instrumentName='openSpectrometer'):

class Biowave_II(spectrometer):
	
    def read(self):
        return "device data goes here"
	
class plotter:
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
		self.button=gtk.Button('Select this point as the peak')
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

class getDataFromSerial:
	def __init__(self, *args, **kwargs):
		self.ser=serial.Serial(*args, **kwargs)
	def close(self):
		self.ser.close()
	def saveDataToSingleFile(self, fileName):
		self.fileName=fileName
	def saveDataToMultipleFiles(self, filenameList):
		self.filenameList=filenameList
	def startSaving(self, enterSampleIds=True):
		print 'Starting to look for data'
		startWavelength=None; endWavelength=None; datapoints=[]; datapointsReady=False; lastLine=''
		while(1):
			try:
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
					sampleID=""
					if enterSampleIds==True:
						print 'enter sample ID'
						sampleID = raw_input().strip()
						while (sampleID==""):
							print 'please re-enter the sample ID'
							sampleID = raw_input()
					f=open(fileName,'a')
					f.write(json.dumps(OrderedDict([("openSpectrometer data file" , "version 0"), ("sample ID", sampleID),	("startWavelength" , startWavelength), 
										("endWavelength" , endWavelength), ("minAbsorbanceDataValue" , minAbs), 
										("maxAbsorbanceDataValue" , maxAbs), ("datapoints" , datapoints)]), sort_keys=False))
					f.close()
					datapointsReady=False
					startWavelength=None; endWavelength=None; datapoints=[]

			except KeyboardInterrupt:
				print 'Done taking samples'
			
import _winreg as winreg
import itertools
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

if __name__ == '__main__':
	if len(sys.argv)>1:
		spectrometer=getDataFromSerial(port=sys.argv[1], baudrate=115200, rtscts=True, dsrdtr=True, timeout=5)
		spectrometer.saveDataToSingleFile('feExperiment'+datetime.datetime.now().strftime("%d_%m_%y__%H_%M_%S"))
		spectrometer.startSaving()
		spectrometer.close()
	else:
		print 'ports available:'
		for i in enumerate_serial_ports():
			print i