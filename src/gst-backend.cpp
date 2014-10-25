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
static void on_pad_added (GstElement *element, GstPad     *pad, gpointer    data) {
    GstPad *sinkpad;
    GstElement *decoder = (GstElement *) data;

    /* We can now link this pad with the vorbis-decoder sink pad */
    g_print ("Dynamic pad created, linking demuxer/decoder\n");

    sinkpad = gst_element_get_static_pad (decoder, "sink");

    gst_pad_link (pad, sinkpad);
    gst_object_unref (sinkpad);
}

void Backend::createBusForMessages() {
    GstBus *bus;
    bus = gst_pipeline_get_bus(GST_PIPELINE(_commands->getPipeline()));
    gst_bus_add_watch(bus, bus_cb, NULL);
    gst_object_unref(bus);
}

void Backend::incrustVideo() {
    if(GST_IS_X_OVERLAY(_commands->getElement("videosink")))	{
        gst_x_overlay_set_xwindow_id(GST_X_OVERLAY(_commands->getElement("videosink")), GPOINTER_TO_INT(_window));
    }
}


void Backend::backend_play(const gchar *filename) {
	backend_stop();
    _commands = new GStreamerCommands();

    _commands->addElement("source", "filesrc");

    _commands->addElement("videosink", "xvimagesink");
    _commands->addElement("demuxer", "decodebin");
    _commands->addElement("audioconverter", "audioconvert");
    _commands->addElement("audio-output", "autoaudiosink");
    _commands->addElement("queue-audio", "queue");
    _commands->addElement("volume", "volume");

    _commands->addElement("queue-video", "queue");
    _commands->addElement("saturation", "videobalance");

    _commands->checkAllElements();

    /* Mise en place du pipeline */
    /* on configurer le nom du fichier a l'element source */
    g_object_set (G_OBJECT (_commands->getElement("source")), "location", filename, NULL);
    g_object_set (G_OBJECT (_commands->getElement("saturation")), "saturation", 0.0, NULL);
    g_object_set (G_OBJECT (_commands->getElement("volume")), "volume", 0.5, NULL);
    g_object_set(G_OBJECT(_commands->getPipeline()), "video-sink", _commands->getElement("videosink"), NULL);
    g_object_set(G_OBJECT(_commands->getElement("videosink")), "force-aspect-ratio", TRUE, NULL);

    createBusForMessages();

    _commands->addAllElements();

    gst_element_link (_commands->getElement("source"), _commands->getElement("demuxer"));
    gst_element_link_many(_commands->getElement("queue-audio"), _commands->getElement("volume"), _commands->getElement("audioconverter"), _commands->getElement("audio-output"), NULL);
    gst_element_link_many (_commands->getElement("queue-video"), _commands->getElement("saturation"), _commands->getElement("videosink"), NULL);
    g_signal_connect (_commands->getElement("demuxer"), "pad-added", G_CALLBACK (on_pad_added), _commands->getElement("queue-audio"));
    g_signal_connect (_commands->getElement("demuxer"), "pad-added", G_CALLBACK (on_pad_added), _commands->getElement("queue-video"));

    incrustVideo();

    gst_element_set_state(_commands->getPipeline(), GST_STATE_PLAYING);
}

void Backend::backend_stop(void) {
    if(_commands != 0 && _commands->getPipeline() != 0) {
        gst_element_set_state(_commands->getPipeline(), GST_STATE_NULL);
        gst_object_unref(GST_OBJECT(_commands->getPipeline()));
        delete _commands;
    }
}

void Backend::backend_pause(void) {
    gst_element_set_state(_commands->getPipeline(), GST_STATE_PAUSED);
}

void Backend::backend_resume(void) {
    gst_element_set_state(_commands->getPipeline(), GST_STATE_PLAYING);
}

void Backend::backend_reset(void) {
    gst_element_seek(_commands->getPipeline(), 1.0,
			GST_FORMAT_TIME,
            _seek_flags,
			GST_SEEK_TYPE_SET, 0,
			GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE);
}

void Backend::backend_seek(gint value) {
    gst_element_seek(_commands->getPipeline(), 1.0,
			GST_FORMAT_TIME,
            _seek_flags,
			GST_SEEK_TYPE_CUR, value * GST_SECOND,
			GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE);
}

void Backend::backend_seek_absolute(guint64 value) {
    gst_element_seek(_commands->getPipeline(), 1.0,
			GST_FORMAT_TIME,
            _seek_flags,
			GST_SEEK_TYPE_SET, value,
			GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE);
}

guint64 Backend::backend_query_position(void) {
	GstFormat format = GST_FORMAT_TIME;
	gint64 cur;
	gboolean result;

    result = gst_element_query_position(_commands->getPipeline(), &format, &cur);
	if(!result || format != GST_FORMAT_TIME)
		return GST_CLOCK_TIME_NONE;

	return cur;
}

guint64 Backend::backend_query_duration(void) {
	GstFormat format = GST_FORMAT_TIME;
	gint64 cur;
	gboolean result;

    result = gst_element_query_duration(_commands->getPipeline(), &format, &cur);
	if(!result || format != GST_FORMAT_TIME)
		return GST_CLOCK_TIME_NONE;

	return cur;
}

Backend::Backend(int *argc, char **argv[]) {
    _seek_flags =(GstSeekFlags)(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT);
    _commands = NULL;
    gst_init(argc, argv);
}
