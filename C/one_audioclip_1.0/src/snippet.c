/*
 * bla bla bla
 *
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>
#include <glib/gprintf.h>
#include <gst/gst.h>
#include <ges/ges.h>

GstElement *pipeline, *sink, *comp, *compconvert, *audio1;

static gboolean bus_call (GstBus *bus, GstMessage *msg, gpointer data)
{
    GMainLoop *loop = (GMainLoop *) data;

    switch (GST_MESSAGE_TYPE (msg))
    {
        case GST_MESSAGE_EOS:
        {
            g_print ("End-of-stream\n");

            g_main_loop_quit (loop);
            break;
        }
        case GST_MESSAGE_ERROR:
        {
            gchar *debug;
            GError *err;

            gst_message_parse_error (msg, &err, &debug);
            g_free (debug);

            g_print ("Error: %s\n", err->message);
            g_error_free (err);

            g_main_loop_quit (loop);
            break;
        }
        case GST_MESSAGE_STATE_CHANGED:
        {
            //g_print ("GST_MESSAGE_STATE_CHANGED: \n");
            break;
        }
        case GST_MESSAGE_SEGMENT_DONE:
        {
            //g_print ("Bus msg: GST_MESSAGE_SEGMENT_DONE\n");
            break;
        }
        default:
        {
            //g_print ("Msg-Type: %d", GST_MESSAGE_TYPE (msg));
            break;
        }
    }

    return TRUE;
}

static void on_pad (GstElement *element, GstPad *pad, gpointer data)
{
    GstPad *sinkpad;
    GstPadLinkReturn result;

    /* We can now link this pad with the  decoder */

    gst_element_get_name (compconvert);
    sinkpad = gst_element_get_compatible_pad (compconvert, pad,
        gst_pad_query_caps (pad, NULL));

    g_print ("linking pads");
    result = gst_pad_link (pad, sinkpad);
    if (result == GST_PAD_LINK_OK)
      g_print ("Dynamic pad created, linking comp/parser\n");
    else
      g_print ("comp_new_pad(): gst_pad_link() failed! result: %d\n", result);

    gst_object_unref (sinkpad);
}

int main (int argc, char *argv[])
{
    GMainLoop *loop;
    GstBus *bus;

    /* initialize GStreamer */
    gst_init (&argc, &argv);
    g_print ("%s\n", gst_version_string());

    loop = g_main_loop_new (NULL, FALSE);
    gst_debug_set_active (TRUE);
    gst_debug_set_threshold_for_name ("*", GST_LEVEL_WARNING);

    /* check input arguments */
    if (argc != 2)
    {
        g_print ("Usage: %s <filename>", argv[0]);
        return - 1;
    }

    /* create the pipeline */
    pipeline = gst_pipeline_new ("gnonlin-test");

    /* creating a gnlcomposition */
    comp = gst_element_factory_make ("gnlcomposition", "mycomposition");
    gst_bin_add (GST_BIN (pipeline), comp);
    g_signal_connect (comp, "pad-added", G_CALLBACK (on_pad), NULL);

    /* create an audioconvert */
    compconvert = gst_element_factory_make ("audioconvert", "compconvert");
    gst_bin_add (GST_BIN (pipeline), compconvert);

    /* create an audiosink */
    sink = gst_element_factory_make ("autoaudiosink", "sink");
    gst_bin_add (GST_BIN (pipeline), sink);

    /* create a gnurisource */
    audio1 = gst_element_factory_make ("gnlurisource", "audio1");
    gst_bin_add (GST_BIN (comp), audio1);

    /* set the gnlurisource properties */
    g_object_set (G_OBJECT (audio1),
        "uri", argv[1],
        "start", 0 * GST_SECOND,
        "duration", 5 * GST_SECOND,
        "media-start", 10 * GST_SECOND,
        "media-duration",6 * GST_SECOND,
        NULL);

    if (! pipeline || ! comp || ! compconvert || ! sink || ! audio1)
    {
        g_print ("One element could not be created\n");
        return - 1;
    }

    bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
    gst_bus_add_watch (bus, bus_call, loop);
    gst_object_unref (bus);

    gst_element_link (compconvert, sink);

    /* Now set to playing and iterate. */
    g_print ("Setting to PLAYING\n");
    gst_element_set_state (pipeline, GST_STATE_PLAYING);
    g_print ("Running\n");

    g_main_loop_run (loop);

    /* clean up nicely */
    g_print ("Returned, stopping playback\n");
    gst_element_set_state (pipeline, GST_STATE_NULL);
    g_print ("Deleting pipeline\n");
    gst_object_unref (GST_OBJECT (pipeline));

    return 0;
}
