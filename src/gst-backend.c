#include <gst/gst.h>
#include <gst/interfaces/xoverlay.h>

#include "gst-backend.h"

static GstElement *pipeline;
static GstElement *videosink;
static gpointer window;
static GstSeekFlags seek_flags =(GstSeekFlags)(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT);

static gboolean bus_cb(GstBus *bus, GstMessage *msg, gpointer data) {
	switch(GST_MESSAGE_TYPE(msg)) {
		case GST_MESSAGE_EOS:
			g_debug("end-of-stream");
			break;
		case GST_MESSAGE_ERROR:
			gchar *debug;
			GError *err;

			gst_message_parse_error(msg, &err, &debug);
			g_free(debug);

			g_warning("Error: %s", err->message);
			g_error_free(err);
			break;
		default:
			break;
	}

	return TRUE;
}

void backend_init(int *argc, char **argv[]) {
	gst_init(argc, argv);
}

void backend_set_window(gpointer window_) {
	window = window_;
}

void backend_play(const gchar *filename) {
	backend_stop();

	pipeline = gst_element_factory_make("playbin", "gst-player");
	videosink = gst_element_factory_make("xvimagesink", "videosink");

	g_object_set(G_OBJECT(pipeline), "video-sink", videosink, NULL);

	{
		GstBus *bus;
		bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
		gst_bus_add_watch(bus, bus_cb, NULL);
		gst_object_unref(bus);
	}

	{
		gchar *uri;

		if(gst_uri_is_valid(filename))
		{
			uri = g_strdup(filename);
		}
		else if(g_path_is_absolute(filename))
		{
			uri = g_filename_to_uri(filename, NULL, NULL);
		}
		else
		{
			gchar *tmp;
			tmp = g_build_filename(g_get_current_dir(), filename, NULL);
			uri = g_filename_to_uri(tmp, NULL, NULL);
			g_free(tmp);
		}

		g_debug("%s", uri);
		g_object_set(G_OBJECT(pipeline), "uri", uri, NULL);
		g_free(uri);
	}

	g_object_set(G_OBJECT(videosink), "force-aspect-ratio", TRUE, NULL);

	if(GST_IS_X_OVERLAY(videosink))
	{
		gst_x_overlay_set_xwindow_id(GST_X_OVERLAY(videosink), GPOINTER_TO_INT(window));
	}

	gst_element_set_state(pipeline, GST_STATE_PLAYING);
}

void backend_stop(void) {
	if(pipeline)
	{
		gst_element_set_state(pipeline, GST_STATE_NULL);
		gst_object_unref(GST_OBJECT(pipeline));
		pipeline = NULL;
	}
}

void backend_pause(void) {
	gst_element_set_state(pipeline, GST_STATE_PAUSED);
}

void backend_resume(void) {
	gst_element_set_state(pipeline, GST_STATE_PLAYING);
}

void backend_reset(void) {
	gst_element_seek(pipeline, 1.0,
			GST_FORMAT_TIME,
			seek_flags,
			GST_SEEK_TYPE_SET, 0,
			GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE);
}

void backend_seek(gint value) {
	gst_element_seek(pipeline, 1.0,
			GST_FORMAT_TIME,
			seek_flags,
			GST_SEEK_TYPE_CUR, value * GST_SECOND,
			GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE);
}

void backend_seek_absolute(guint64 value) {
	gst_element_seek(pipeline, 1.0,
			GST_FORMAT_TIME,
			seek_flags,
			GST_SEEK_TYPE_SET, value,
			GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE);
}

guint64 backend_query_position(void) {
	GstFormat format = GST_FORMAT_TIME;
	gint64 cur;
	gboolean result;

	result = gst_element_query_position(pipeline, &format, &cur);
	if(!result || format != GST_FORMAT_TIME)
		return GST_CLOCK_TIME_NONE;

	return cur;
}

guint64 backend_query_duration(void) {
	GstFormat format = GST_FORMAT_TIME;
	gint64 cur;
	gboolean result;

	result = gst_element_query_duration(pipeline, &format, &cur);
	if(!result || format != GST_FORMAT_TIME)
		return GST_CLOCK_TIME_NONE;

	return cur;
}

void backend_deinit(void) {
}
