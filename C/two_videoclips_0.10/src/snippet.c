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

GstElement *pipeline, *conv, *sink, *comp, *gnlfilesource1, *gnlfilesource2;

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

static void comp_new_pad (GstElement *element, GstPad *pad, gpointer data)
{
    GstPad *sinkpad;
    /* We can now link this pad with the  decoder */

    // sinkpad = gst_element_get_pad (parser, "sink");
    // do we have toask for a compatible pad ?
    sinkpad = gst_element_get_compatible_pad (conv, pad, gst_pad_get_caps (pad));

    gchar* srcCapsStr = gst_caps_to_string (gst_pad_get_caps (pad));
    gchar* sinkCapsStr = gst_caps_to_string (gst_pad_get_caps (sinkpad));
    //
    GstPadLinkReturn result = gst_pad_link (pad, sinkpad);
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
    loop = g_main_loop_new (NULL, FALSE);
    gst_debug_set_active (TRUE);
    gst_debug_set_threshold_for_name ("*", GST_LEVEL_WARNING);

    /* check input arguments */
    if (argc != 3)
    {
        g_print ("Usage: %s <filename1> <filename2>\n", argv[0]);
        return - 1;
    }

    /* create elements */
    pipeline = gst_pipeline_new ("TM_video-player");
    conv = gst_element_factory_make ("ffmpegcolorspace", "ffmpeg-colorspace");
    sink = gst_element_factory_make ("autovideosink", "directdrawsink-output");
    comp = gst_element_factory_make ("gnlcomposition", "mycomposition");
    gnlfilesource1 = gst_element_factory_make ("gnlfilesource", "video1");
    gnlfilesource2 = gst_element_factory_make ("gnlfilesource", "video2");
    gst_bin_add (GST_BIN (comp), gnlfilesource1);
    gst_bin_add (GST_BIN (comp), gnlfilesource2);

    g_object_set (G_OBJECT (gnlfilesource1),
        "location", argv[1],
        "start", 0 * GST_SECOND,
        "duration", 10 * GST_SECOND,
        "media-start", 0 * GST_SECOND,
        "media-duration", 10 * GST_SECOND,
        NULL);
    g_object_set (G_OBJECT (gnlfilesource2),
        "location", argv[2],
        "start", 10 * GST_SECOND,
        "duration", 10 * GST_SECOND,
        "media-start", 20 * GST_SECOND,
        "media-duration", 10 * GST_SECOND,
        NULL);

    if (! pipeline || ! conv || ! sink || ! comp)
    {
        g_print ("One element could not be created\n");
        return - 1;
    }

    bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
    gst_bus_add_watch (bus, bus_call, loop);
    gst_object_unref (bus);

    /* put all elements in a bin */
    gst_bin_add_many (GST_BIN (pipeline), comp, conv, sink, NULL);

    gst_element_link (conv, sink);

    g_signal_connect (comp, "pad-added", G_CALLBACK (comp_new_pad), NULL);

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
