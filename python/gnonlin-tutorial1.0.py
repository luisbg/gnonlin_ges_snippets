#!/usr/bin/python
from gi.repository import Gst

import pygtk
import gtk
import gtk.glade

# original source:
#	http://www.jonobacon.org/2006/12/27/using-gnonlin-with-gstreamer-and-python/


class GnonlinTutorial:
	def __init__(self):

		# GObject.threads_init()
		Gst.init(None)

		# set up the glade file
		self.wTree = gtk.glade.XML("gui.glade", "mainwindow")
		
		signals = {
			"on_play_clicked" : self.OnPlay,
			"on_stop_clicked" : self.OnStop,
			"on_quit_clicked" : self.OnQuit,
		}

		self.wTree.signal_autoconnect(signals)

		# creating the pipeline
		self.pipeline = Gst.Pipeline("mypipeline")

		# creating a gnlcomposition
		self.comp = Gst.ElementFactory.make("gnlcomposition", "mycomposition")
		self.pipeline.add(self.comp)
		self.comp.connect("pad-added", self.OnPad)

		# create an audioconvert
		self.compconvert = Gst.ElementFactory.make("audioconvert", "compconvert")
		self.pipeline.add(self.compconvert)

		# create an alsasink
		self.sink = Gst.ElementFactory.make("autoaudiosink", "autoaudiosink")
		self.pipeline.add(self.sink)
		self.compconvert.link(self.sink)
		
		# create a gnlfilesource
		self.audio1 = Gst.ElementFactory.make("gnlurisource", "audio1")
		self.comp.add(self.audio1)

		# set the gnlfilesource properties
		self.audio1.set_property("uri", "file:///home/luisbg/tst.mp3")
		self.audio1.set_property("start", 0 * Gst.SECOND)
		self.audio1.set_property("duration", 5 * Gst.SECOND)
		self.audio1.set_property("media-start", 10 * Gst.SECOND)
		self.audio1.set_property("media-duration", 5 * Gst.SECOND)

		# show the window
		self.window = self.wTree.get_widget("mainwindow")
		self.window.show_all()

	def OnPad(self, comp, pad):
		print "pad '" + pad.get_name() + "' added in " + comp.get_property("name")
		print pad.get_name()
		convpad = self.compconvert.get_compatible_pad(pad, pad.query_caps(None))
		pad.link(convpad)

	def OnPlay(self, widget):
		print "play"
		self.pipeline.set_state(Gst.State.PLAYING)

	def OnStop(self, widget):
		print "stop"
		self.pipeline.set_state(Gst.State.NULL)

	def OnQuit(self, widget):
		print "quitting"
		gtk.main_quit()

start = GnonlinTutorial()
gtk.main()
