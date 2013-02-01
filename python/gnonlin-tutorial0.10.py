#!/usr/bin/python

import pygst
pygst.require("0.10")
import gst
import pygtk
import gtk
import gtk.glade

# original source: 
#	http://www.jonobacon.org/2006/12/27/using-gnonlin-with-gstreamer-and-python/

class Main:
	def __init__(self):

		# set up the glade file
		self.wTree = gtk.glade.XML("gui.glade", "mainwindow")
		
		signals = {
			"on_play_clicked" : self.OnPlay,
			"on_stop_clicked" : self.OnStop,
			"on_quit_clicked" : self.OnQuit,
		}

		self.wTree.signal_autoconnect(signals)

		# creating the pipeline
		self.pipeline = gst.Pipeline("mypipeline")

		# creating a gnlcomposition
		self.comp = gst.element_factory_make("gnlcomposition", "mycomposition")
		self.pipeline.add(self.comp)
		self.comp.connect("pad-added", self.OnPad)

		# create an audioconvert
		self.compconvert = gst.element_factory_make("audioconvert", "compconvert")
		self.pipeline.add(self.compconvert)

		# create an alsasink
		self.sink = gst.element_factory_make("alsasink", "alsasink")
		self.pipeline.add(self.sink)
		self.compconvert.link(self.sink)
		
		# create a gnlfilesource
		self.audio1 = gst.element_factory_make("gnlfilesource", "audio1")
		self.comp.add(self.audio1)

		# set the gnlfilesource properties
		self.audio1.set_property("location", "/home/luisbg/tst.mp3")
		self.audio1.set_property("start", 0 * gst.SECOND)
		self.audio1.set_property("duration", 5 * gst.SECOND)
		self.audio1.set_property("media-start", 10 * gst.SECOND)
		self.audio1.set_property("media-duration", 5 * gst.SECOND)

		# show the window
		self.window = self.wTree.get_widget("mainwindow")
		self.window.show_all()

	def OnPad(self, comp, pad):
		print "pad added!"
		convpad = self.compconvert.get_compatible_pad(pad, pad.get_caps())
		pad.link(convpad)

	def OnPlay(self, widget):
		print "play"
		self.pipeline.set_state(gst.STATE_PLAYING)

	def OnStop(self, widget):
		print "stop"
		self.pipeline.set_state(gst.STATE_NULL)

	def OnQuit(self, widget):
		print "quitting"
		gtk.main_quit()

start=Main()
gtk.main()
