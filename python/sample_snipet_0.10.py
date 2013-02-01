import sys, os, random, threading
import gst, gobject, sys, gtk

# original source:
# http://snipdom.net/gstsnips/example_gnlcomposition.py/

gobject.threads_init()

pipeline = gst.Pipeline()
adder = gst.element_factory_make("adder")

PAD_MUTEX = threading.Lock()

def on_pad(gnlcomp, pad):
    print "on_pad"
    PAD_MUTEX.acquire()
    convpad = adder.get_compatible_pad(pad, pad.get_caps())        
    pad.link(convpad)
    PAD_MUTEX.release()
    print "padded", convpad

def handle_message(bus, message):
    if message.type == gst.MESSAGE_EOS:
        print "trying to quit"
        #pipeline.set_state(gst.STATE_NULL)
        gtk.main_quit()
        sys.exit(0)

bus = pipeline.get_bus()
bus.enable_sync_message_emission()
bus.connect("sync-message", handle_message)

def get_comp(path, start, duration):
    """returns a gnlcomposition with the file at some position in a sparse timeline"""

    comp = gst.element_factory_make("gnlcomposition")

    # setup silent backing track as default
    silence = gst.element_factory_make("audiotestsrc")
    silence.set_property("volume", 0.0)
    silencesrc = gst.element_factory_make("gnlsource")
    silencesrc.set_property("priority", 2**32-1) # -1 is not of type guint!
    silencesrc.add(silence)
    comp.add(silencesrc)

    #add file
    filesrc = gst.element_factory_make("gnlurisource")
    filesrc.set_property("uri", "file://%s" % (path))
    filesrc.set_property("start", int(start*gst.SECOND))
    filesrc.set_property("duration", int(duration*gst.SECOND))
    # strange errors without media-start & duration
    filesrc.set_property("media-start", 0)
    filesrc.set_property("media-duration", int(duration*gst.SECOND))

    comp.add(filesrc)

    #set on-pad behavior
    comp.connect("pad-added", on_pad)

    return comp

many = []
for i in range(3):
    comp = get_comp("/home/luisbg/tst.mp3", i, 5)
    many.append(comp)

convert = gst.element_factory_make("audioconvert")
resample = gst.element_factory_make("audioresample")

caps = gst.Caps("audio/x-raw-int, channels=1, width=16, rate=48000")
filt = gst.element_factory_make("capsfilter")
filt.set_property("caps", caps)

enc = gst.element_factory_make("wavenc")

#sink = gst.element_factory_make("alsasink")

sink = gst.element_factory_make("filesink")
outloc = "out.wav" #os.path.join(bundle.dir, "punch.wav")
sink.set_property("location", outloc)

pipeline.add(*many)
pipeline.add(adder, resample, convert, filt, enc, sink)
gst.element_link_many(adder, resample, convert, filt, enc, sink)

pipeline.set_state(gst.STATE_PLAYING)
gtk.main()
