
#include "gst-backend.h"

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

void Backend::backend_set_window(gpointer window_) {
    _window = window_;
}

void Backend::backend_play(const gchar *filename) {
	backend_stop();

    _pipeline = gst_element_factory_make("playbin", "gst-player");
    _videosink = gst_element_factory_make("xvimagesink", "_videosink");

    g_object_set(G_OBJECT(_pipeline), "video-sink", _videosink, NULL);

	{
		GstBus *bus;
        bus = gst_pipeline_get_bus(GST_PIPELINE(_pipeline));
		gst_bus_add_watch(bus, bus_cb, NULL);
		gst_object_unref(bus);
	}

	{
		gchar *uri;

        if(gst_uri_is_valid(filename)) {
			uri = g_strdup(filename);
        } else if(g_path_is_absolute(filename))	{
			uri = g_filename_to_uri(filename, NULL, NULL);
        } else {
			gchar *tmp;
			tmp = g_build_filename(g_get_current_dir(), filename, NULL);
			uri = g_filename_to_uri(tmp, NULL, NULL);
			g_free(tmp);
		}

		g_debug("%s", uri);
        g_object_set(G_OBJECT(_pipeline), "uri", uri, NULL);
		g_free(uri);
	}

    g_object_set(G_OBJECT(_videosink), "force-aspect-ratio", TRUE, NULL);

    if(GST_IS_X_OVERLAY(_videosink))	{
        gst_x_overlay_set_xwindow_id(GST_X_OVERLAY(_videosink), GPOINTER_TO_INT(_window));
	}

    gst_element_set_state(_pipeline, GST_STATE_PLAYING);
}

void Backend::backend_stop(void) {
    if(_pipeline) {
        gst_element_set_state(_pipeline, GST_STATE_NULL);
        gst_object_unref(GST_OBJECT(_pipeline));
        _pipeline = NULL;
	}
}

void Backend::backend_pause(void) {
    gst_element_set_state(_pipeline, GST_STATE_PAUSED);
}

void Backend::backend_resume(void) {
    gst_element_set_state(_pipeline, GST_STATE_PLAYING);
}

void Backend::backend_reset(void) {
    gst_element_seek(_pipeline, 1.0,
			GST_FORMAT_TIME,
            _seek_flags,
			GST_SEEK_TYPE_SET, 0,
			GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE);
}

void Backend::backend_seek(gint value) {
    gst_element_seek(_pipeline, 1.0,
			GST_FORMAT_TIME,
            _seek_flags,
			GST_SEEK_TYPE_CUR, value * GST_SECOND,
			GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE);
}

void Backend::backend_seek_absolute(guint64 value) {
    gst_element_seek(_pipeline, 1.0,
			GST_FORMAT_TIME,
            _seek_flags,
			GST_SEEK_TYPE_SET, value,
			GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE);
}

guint64 Backend::backend_query_position(void) {
	GstFormat format = GST_FORMAT_TIME;
	gint64 cur;
	gboolean result;

    result = gst_element_query_position(_pipeline, &format, &cur);
	if(!result || format != GST_FORMAT_TIME)
		return GST_CLOCK_TIME_NONE;

	return cur;
}

guint64 Backend::backend_query_duration(void) {
	GstFormat format = GST_FORMAT_TIME;
	gint64 cur;
	gboolean result;

    result = gst_element_query_duration(_pipeline, &format, &cur);
	if(!result || format != GST_FORMAT_TIME)
		return GST_CLOCK_TIME_NONE;

	return cur;
}

Backend::Backend(int *argc, char **argv[]) {
    _seek_flags =(GstSeekFlags)(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT);
    gst_init(argc, argv);
}
