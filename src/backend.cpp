#include "backend.h"

gboolean Backend::bus_cb(GstBus *bus, GstMessage *msg, gpointer data) {
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

    return true;
}

void Backend::on_pad_added (GstElement *element, GstPad     *pad, gpointer    data) {
    GstPad *sinkpad;
    GstElement *decoder = (GstElement *) data;

    /* We can now link this pad with the vorbis-decoder sink pad */
    g_print ("Dynamic pad created, linking demuxer/decoder\n");

    sinkpad = gst_element_get_static_pad (decoder, "sink");

    gst_pad_link (pad, sinkpad);
    gst_object_unref (sinkpad);
}

void Backend::setWindow(gpointer window_) {
    _window = window_;
}

void Backend::stop() {
    pause();
    seek(0);
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
double Backend::getCurrentVolume() const {
    return _currentVolume;
}

void Backend::setCurrentVolume(double currentVolume) {
    _currentVolume = currentVolume;
}


void Backend::setVolume(const double volume) {
    _currentVolume = volume;
    _commands->setElement("volume", "volume", volume);

}
void Backend::volumeUp() {
    setVolume(_currentVolume+0.20 < 2 ? _currentVolume + 0.20 : _currentVolume);
}

void Backend::volumeDown() {
    setVolume(_currentVolume > 0 ? _currentVolume - 0.20 : 0);
}

void Backend::play(const std::string filename, const std::string srtfilename) {
    quit();
    _commands = new GStreamerCommands();
    _subtitlesIsHidding = srtfilename == "";
    if(!_subtitlesIsHidding) { // We have a subtitle
        _commands->addElement ("subOverlay",    "subtitleoverlay");
        _commands->addElement ("subSource",          "filesrc");
        _commands->addElement ("subParse",          "subparse");

        _commands->setElement("subSource", "location", srtfilename);
        _commands->setElement("subOverlay", "silent", false);
    }

    _commands->addElement("source", "filesrc");
    _commands->addElement("demuxer", "oggdemux");
    _commands->addElement("audioQueue", "queue");
    _commands->addElement("volume", "volume");
    _commands->addElement("videoQueue", "queue");
    _commands->addElement("audioDecoder", "vorbisdec");
    _commands->addElement("videoDecoder", "theoradec");
    _commands->addElement("audioConv", "audioconvert");
    _commands->addElement("videoConv", "ffmpegcolorspace");
    _commands->addElement("videosink", "xvimagesink");
    _commands->addElement("audiosink", "autoaudiosink");

    _commands->checkAllElements();
    _commands->setElement("source", "location", filename);
    setVolume(0.5);
    _commands->addAllElements();
    createBusForMessages();

    gst_element_link (_commands->getElement("source"), _commands->getElement("demuxer"));

    if(!_subtitlesIsHidding) {
        gst_element_link_many (_commands->getElement("videoQueue"), _commands->getElement("videoDecoder"),
                               _commands->getElement("videoConv"), _commands->getElement("subOverlay"),
                               _commands->getElement("videosink"), NULL);
    } else {
        gst_element_link_many (_commands->getElement("videoQueue"), _commands->getElement("videoDecoder"),
                               _commands->getElement("videoConv"),
                               _commands->getElement("videosink"), NULL);
    }

    gst_element_link_many (_commands->getElement("audioQueue"), _commands->getElement("audioDecoder"),
                           _commands->getElement("audioConv"), _commands->getElement("volume"),_commands->getElement("audiosink"), NULL);

    g_signal_connect (_commands->getElement("demuxer"), "pad-added", G_CALLBACK (on_pad_added), _commands->getElement("audioQueue"));
    g_signal_connect (_commands->getElement("demuxer"), "pad-added", G_CALLBACK (on_pad_added), _commands->getElement("videoQueue"));

    if(srtfilename != "") { // We have a subtitle
        /* Linking subtitles and video pads together */
        gst_element_link (_commands->getElement("subSource"), _commands->getElement("subParse"));

        if(!gst_element_link_pads(_commands->getElement("subParse"), NULL, _commands->getElement("subOverlay"), NULL)) {
            g_printerr("Pads couldn't be linked\n");
        }
    }

    incrustVideo();

    gst_element_set_state(_commands->getPipeline(), GST_STATE_PLAYING);
}

void Backend::quit(void) {
    if(_commands != 0 && _commands->getPipeline() != 0) {
        gst_element_set_state(_commands->getPipeline(), GST_STATE_NULL);
        delete _commands;
    }
}

void Backend::pause(void) {
    gst_element_set_state(_commands->getPipeline(), GST_STATE_PAUSED);
}

void Backend::resume(void) {
    gst_element_set_state(_commands->getPipeline(), GST_STATE_PLAYING);
}

void Backend::reset(std::string filename, std::string srtfilename) {
    play(filename, srtfilename);
}
void Backend::showSubtitles(void) {
    _commands->setElement("subOverlay", "silent", false);

    _subtitlesIsHidding = false;
}

void Backend::hideSubtitles(void) {
    _commands->setElement("subOverlay", "silent", true);

    _subtitlesIsHidding = true;
}

bool Backend::subtitlesIsHiding(void) {
    return _subtitlesIsHidding;
}

void Backend::seek(gint value) {
    gst_element_seek(_commands->getPipeline(), 1.0,
			GST_FORMAT_TIME,
            _seek_flags,
			GST_SEEK_TYPE_CUR, value * GST_SECOND,
			GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE);
}

void Backend::absoluteSeek(unsigned long value) {
    gst_element_seek(_commands->getPipeline(), 1.0,
			GST_FORMAT_TIME,
            _seek_flags,
			GST_SEEK_TYPE_SET, value,
			GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE);
}

unsigned long Backend::queryPosition(void) {
	GstFormat format = GST_FORMAT_TIME;
	gint64 cur;
    bool result;

    result = gst_element_query_position(_commands->getPipeline(), &format, &cur);
	if(!result || format != GST_FORMAT_TIME)
		return GST_CLOCK_TIME_NONE;

	return cur;
}

unsigned long Backend::queryDuration(void) {
	GstFormat format = GST_FORMAT_TIME;
	gint64 cur;
    bool result;

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



