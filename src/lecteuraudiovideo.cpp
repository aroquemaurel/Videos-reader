#include <gst/gst.h> 
#include <glib.h>
static bool bus_call (GstBus *bus, GstMessage *msg, gpointer data) {
	GMainLoop *loop = (GMainLoop *) data;
	switch (GST_MESSAGE_TYPE (msg)) {

		case GST_MESSAGE_EOS:
			g_print ("End of stream\n");
			g_main_loop_quit (loop);
			break;
		case GST_MESSAGE_ERROR: {
									gchar  *debug;
									GError *error;

									gst_message_parse_error (msg, &error, &debug);
									g_free (debug);

									g_printerr ("Error: %s\n", error->message);
									g_error_free (error);

									g_main_loop_quit (loop);
									break;
								}
		default:
								break;
	}
	return true;
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

int main (int   argc, char *argv[]) {
	GMainLoop *loop;

	GstElement *pipeline, *source, *demuxer,  *conv, *sink, *queueAudio;
	GstElement *autovideosink, *queueVideo, *videoBalance,*volume;
	GstBus *bus;

	/* Initialisation */
	gst_init (&argc, &argv);

	loop = g_main_loop_new (NULL, false);

	/* Verification des arguments d'entree */
	if (argc != 2) {
		g_printerr ("Usage: %s <Ogg/Vorbis filename>\n", argv[0]);
		return -1;
	}

	/* Creation des elements gstreamer */
	pipeline = gst_pipeline_new ("audiovideo-player");
	source   = gst_element_factory_make ("filesrc",       "file-source");
	demuxer  = gst_element_factory_make ("decodebin",      "decodebin");

	// Audio
	conv     = gst_element_factory_make ("audioconvert",  "converter");
	sink     = gst_element_factory_make ("autoaudiosink", "audio-output");
	queueAudio = gst_element_factory_make ("queue", "queue-audio");
	volume = gst_element_factory_make ("volume", "volume");
	// Video
	autovideosink = gst_element_factory_make ("autovideosink", "autovideosink");
	queueVideo = gst_element_factory_make ("queue", "queue-video");
	videoBalance = gst_element_factory_make ("videobalance", "saturation");

	if (!pipeline || !source || !demuxer || !conv || !videoBalance ||
			!sink ||!queueVideo || ! queueAudio || !autovideosink ) {
		g_printerr ("One element could not be created. Exiting.\n");
		return -1;
	}

	/* Mise en place du pipeline */
	/* on configurer le nom du fichier a l'element source */
	g_object_set (G_OBJECT (source), "location", argv[1], NULL);
	g_object_set (G_OBJECT (videoBalance), "saturation", 0.0, NULL);
	g_object_set (G_OBJECT (volume), "volume", 0.5, NULL);

	/* on rajoute une gestion de messages */
	bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
	gst_bus_add_watch (bus, bus_call, loop);
	gst_object_unref (bus);

	/* on rajoute tous les elements dans le pipeline */
	/* file-source | ogg-demuxer | vorbis-decoder | converter | alsa-output */
	gst_bin_add_many (GST_BIN (pipeline),
			source, volume, demuxer, queueAudio, videoBalance, conv, sink, queueVideo, autovideosink, NULL);

	/* On relie les elements entre eux */
	/* file-source -> ogg-demuxer ~> vorbis-decoder -> converter -> alsa-output */
	gst_element_link (source, demuxer);
	gst_element_link_many(queueAudio, volume, conv, sink, NULL);
	gst_element_link_many (queueVideo, videoBalance, autovideosink, NULL);
	g_signal_connect (demuxer, "pad-added", G_CALLBACK (on_pad_added), queueAudio);
	g_signal_connect (demuxer, "pad-added", G_CALLBACK (on_pad_added), queueVideo);

	/* Notez que le demuxer va etre lie au decodeur dynamiquement.
	   la raison est que Ogg peut contenir plusieurs flux (par exemple
	   audio et video). Les connecteurs sources seront crees quand la
	   lecture debutera, par le demuxer quand il detectera le nombre et
	   la nature des flux. Donc nous connectons une fonction de rappel
	   qui sera execute quand le "pad-added" sera emis. */

	/* passage a l'etat "playing" du pipeline */
	g_print ("Lecture de : %s\n", argv[1]);
	gst_element_set_state (pipeline, GST_STATE_PLAYING);

	/* Iteration */
	g_print ("En cours...\n");
	g_main_loop_run (loop);

	/* En dehors de la boucle principale, on nettoie proprement */
	g_print ("Arret de la lecture\n");
	gst_element_set_state (pipeline, GST_STATE_NULL);
	g_print ("Suppression du pipeline\n");
	gst_object_unref (GST_OBJECT (pipeline));

	return EXIT_SUCCESS;
} 
