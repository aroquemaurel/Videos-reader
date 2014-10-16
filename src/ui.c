#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <gdk/gdkkeysyms.h>

#include <string.h>

#include "ui.h"
#include "gst-backend.h"

static gchar *filename;
static GtkWidget *video_output;
static GtkWidget *pause_button;
static GtkWidget *scale;
static guint64 duration;
static GtkWindow *window;
static GtkWidget *controls;

#define DURATION_IS_VALID(x) (x != 0 && x != (guint64) -1)

Ui::Ui() {

}
void Ui::toggle_paused (void) {
	static gboolean paused = FALSE;
	if (paused) {
		backend_resume ();
		gtk_button_set_label (GTK_BUTTON (pause_button), "Pause");
		paused = FALSE;
	} else {
		backend_pause ();
		gtk_button_set_label (GTK_BUTTON (pause_button), "Resume");
		paused = TRUE;
	}
}

void Ui::toggle_fullscreen (void) {
	if (gdk_window_get_state (GTK_WIDGET (window)->window) == GDK_WINDOW_STATE_FULLSCREEN) {
		gtk_window_unfullscreen (window);
		gtk_widget_show (controls);
	} else {
		gtk_window_fullscreen (window);
		gtk_widget_hide (controls);
	}
}

void Ui::pause_cb (GtkWidget *widget, gpointer data) {
	toggle_paused();
}

void Ui::reset_cb (GtkWidget *widget, gpointer data) {
	backend_reset ();
}

gboolean Ui::delete_event (GtkWidget *widget, GdkEvent *event, gpointer data) {
	backend_stop ();
	return FALSE;
}

void Ui::destroy (GtkWidget *widget, gpointer data) {
	gtk_main_quit ();
}

gboolean Ui::key_press (GtkWidget *widget, GdkEventKey *event, gpointer data) {
	switch (event->keyval) {
		case GDK_P:
		case GDK_p:
		case GDK_space:
			Ui::toggle_paused ();
			break;
		case GDK_F11:
		case GDK_F:
		case GDK_f:
		toggle_fullscreen ();
			break;
		case GDK_R:
		case GDK_r:
			backend_reset ();
			break;
		case GDK_Right:
			backend_seek (10);
			break;
		case GDK_Left:
			backend_seek (-10);
			break;
		case GDK_Escape:
		case GDK_Q:
		case GDK_q:
			gtk_main_quit ();
			break;
		default:
			break;
	}

	return TRUE;
}

	static void
seek_cb (GtkRange *range,
		GtkScrollType scroll,
		gdouble value,
		gpointer data)
{
	guint64 to_seek;

	if (!DURATION_IS_VALID (duration))
		duration = backend_query_duration ();

	if (!DURATION_IS_VALID (duration))
		return;

	to_seek = (value / 100) * duration;

#if 0
	g_print ("value: %f\n", value);
	g_print ("duration: %llu\n", duration);
	g_print ("seek: %llu\n", to_seek);
#endif

	backend_seek_absolute (to_seek);
}

void Ui::realize_cb (GtkWidget * widget, gpointer data) {
	GdkWindow *window = gtk_widget_get_window (video_output);

	gdk_window_ensure_native (window);
	backend_set_window (GINT_TO_POINTER (GDK_WINDOW_XID (window)));
	Ui::toggle_fullscreen ();
}

void Ui::start (void) {
	GtkWidget *button;
	GtkWidget *vbox;

	window = GTK_WINDOW (gtk_window_new (GTK_WINDOW_TOPLEVEL));

	g_signal_connect (G_OBJECT (window), "delete_event", G_CALLBACK (delete_event), NULL);
	g_signal_connect (G_OBJECT (window), "destroy", G_CALLBACK (Ui::destroy), NULL);
	g_signal_connect (G_OBJECT (window), "key-press-event", G_CALLBACK (key_press), NULL);

	gtk_container_set_border_width (GTK_CONTAINER (window), 0);

	vbox = gtk_vbox_new (FALSE, 0);

	gtk_container_add (GTK_CONTAINER (window), vbox);

	controls = gtk_hbox_new (FALSE, 0);

	gtk_box_pack_end (GTK_BOX (vbox), controls, FALSE, FALSE, 2);

	{
		GdkColor color;

		gdk_color_parse ("black", &color);
		video_output = gtk_drawing_area_new ();
		gtk_widget_modify_bg (GTK_WIDGET (video_output), GTK_STATE_NORMAL, &color);
		gtk_widget_set_double_buffered (video_output, FALSE);

		gtk_box_pack_start (GTK_BOX (vbox), video_output, TRUE, TRUE, 0);

		gtk_widget_set_size_request (video_output, 0x200, 0x100);

		g_signal_connect (video_output, "realize", G_CALLBACK (realize_cb), NULL);
	}

	{
		button = gtk_button_new_with_label ("Pause");

		g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (pause_cb), NULL);

		gtk_box_pack_start (GTK_BOX (controls), button, FALSE, FALSE, 2);
		pause_button = button;
	}

	{
		button = gtk_button_new_with_label ("Reset");

		g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (reset_cb), NULL);

		gtk_box_pack_start (GTK_BOX (controls), button, FALSE, FALSE, 2);
	}

	{
		GtkObject *adjustment;
		adjustment = gtk_adjustment_new (0, 0, 101, 1, 5, 1);
		scale = gtk_hscale_new (GTK_ADJUSTMENT (adjustment));

		gtk_box_pack_end (GTK_BOX (controls), scale, TRUE, TRUE, 2);

		g_signal_connect (G_OBJECT (scale), "change-value", G_CALLBACK (seek_cb), NULL);
	}

	gtk_widget_show_all (GTK_WIDGET (window));
}

static gboolean timeout (gpointer data) {
	guint64 pos;

	pos = backend_query_position ();
	if (!DURATION_IS_VALID (duration))
		duration = backend_query_duration ();

	if (!DURATION_IS_VALID (duration))
		return TRUE;

#if 0
	g_debug ("duration=%f", duration / ((double) 60 * 1000 * 1000 * 1000));
	g_debug ("position=%llu", pos);
#endif

	/** @todo use events for seeking instead of checking for bad positions. */
	if (pos != 0) {
		double value;
		value = (pos * (((double) 100) / duration));
		gtk_range_set_value (GTK_RANGE (scale), value);
	}

	return TRUE;
}
void Ui::setFileName(std::string s) {
	_fileName = s;
}

std::string Ui::getFileName() {
	return _fileName;
}

gboolean Ui::init (gpointer data) {
	if (filename)
		backend_play (filename);

	g_timeout_add (1000, timeout, NULL);

	return FALSE;
}

int main (int argc, char *argv[]) {
	Ui ui = Ui();
	gtk_init(&argc, &argv);
	backend_init(&argc, &argv);

	ui.start();

	if (argc > 1) {
		filename = g_strdup (argv[1]);
	}

	g_idle_add(Ui::init, NULL);

	gtk_main();

	g_free(filename);

	backend_deinit();

	return 0;
}
