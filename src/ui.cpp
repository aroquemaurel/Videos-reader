#include "ui.h"
#include "gst-backend.h"
#include "player.h"
#include <sstream>
#include <iomanip>

static GtkWidget *video_output;
static GtkWidget *pause_button;
static GtkWidget *play_button;
static GtkWidget *scale;
static guint64 duration;
static GtkWindow *window;
static GtkWidget *controls;
static GtkWidget *subtitles_button;
static GtkWidget *stop_button;
static GtkWidget *labelProgression;

#define DURATION_IS_VALID(x) (x != 0 && x != (guint64) -1)
Ui* Ui::_ui = 0;
Backend* Ui::_back = 0;
GtkWidget* Ui::menu_bar = 0;

Ui::Ui() {
    Ui::_ui = this;
}

Ui::Ui(std::string filepath, std::string srtfilename)
{
    Ui::_ui = this;
    _fileName = filepath;
    _srtFilename = srtfilename;
}

void Ui::toggle_paused (void) {
    static bool paused = false;
	if (paused) {
        _back->backend_resume ();
        gtk_widget_hide(play_button);
        gtk_widget_show(pause_button);
        paused = false;
	} else {
        _back->backend_pause ();
        gtk_widget_hide(pause_button);
        gtk_widget_show(play_button);
        paused = true;
	}
}

void Ui::toggle_fullscreen (void) {
	if (gdk_window_get_state (GTK_WIDGET (window)->window) == GDK_WINDOW_STATE_FULLSCREEN) {
		gtk_window_unfullscreen (window);
		gtk_widget_show (controls);
        gtk_widget_show(menu_bar);
	} else {
		gtk_window_fullscreen (window);
		gtk_widget_hide (controls);
        gtk_widget_hide(menu_bar);
	}
}

void Ui::pause_cb (GtkWidget *widget, gpointer data) {
	_ui->toggle_paused();
}

void Ui::stop_cb(GtkWidget *widget, gpointer data) {
    _back->stop();
    gtk_range_set_value (GTK_RANGE (scale), 0);
}

void Ui::reset_cb (GtkWidget *widget, gpointer data) {
    _back->backend_reset (_ui->getFileName(), _ui->getSrtFilename());
}

gboolean Ui::delete_event (GtkWidget *widget, GdkEvent *event, gpointer data) {
    _back->backend_stop ();
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
			_ui->toggle_paused ();
			break;
		case GDK_F11:
		case GDK_F:
		case GDK_f:
			_ui->toggle_fullscreen ();
			break;
		case GDK_R:
		case GDK_r:
            _back->backend_reset (_ui->getFileName(), _ui->getSrtFilename());
			break;
		case GDK_Right:
            _back->backend_seek (10);
			break;
		case GDK_Left:
            _back->backend_seek (-10);
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

void Ui::seek_cb (GtkRange *range, GtkScrollType scroll, gdouble value, gpointer data)
{
	guint64 to_seek;

	if (!DURATION_IS_VALID (duration))
        duration = _back->backend_query_duration ();

	if (!DURATION_IS_VALID (duration))
		return;

	to_seek = (value / 100) * duration;

    _back->backend_seek_absolute (to_seek);
}

void Ui::realize_cb (GtkWidget * widget, gpointer data) {
	GdkWindow *window = gtk_widget_get_window (video_output);

	gdk_window_ensure_native (window);
    _back->backend_set_window (GINT_TO_POINTER (GDK_WINDOW_XID (window)));
}

void Ui::subtitles_cb(GtkWidget * widget, gpointer data) {
    if(_back->subtitlesIsHiding()) {
        _back->showSubtitles();
        gtk_button_set_label(GTK_BUTTON(subtitles_button), "Hide srt");
    } else {
        _back->hideSubtitles();
        gtk_button_set_label(GTK_BUTTON(subtitles_button), "Show srt");
    }
}
void Ui::createMenus(GtkWidget *vbox)
{
    GtkWidget *menuFile;
    GtkWidget *menuVideo;
    GtkWidget *fileLabel;
    GtkWidget *menuFile_items;
    GtkWidget *menuVideo_items;
    GtkWidget *videoLabel;

    //***//
    {

        /* Init the menu-widget, and remember -- never
         * gtk_show_widget() the menu widget!! */
        menuFile = gtk_menu_new();
        menuVideo = gtk_menu_new();

        fileLabel = gtk_menu_item_new_with_label("Fichier");
        gtk_widget_show(fileLabel);
        menuFile_items = gtk_menu_item_new_with_label("Ouvrir un fichier");
        gtk_menu_append(GTK_MENU (menuFile), menuFile_items);
        menuFile_items = gtk_menu_item_new_with_label("Quitter");
        gtk_widget_show(menuFile_items);
        gtk_menu_append(GTK_MENU (menuFile), menuFile_items);


        videoLabel = gtk_menu_item_new_with_label("Vidéo");
        gtk_widget_show(videoLabel);
        menuVideo_items = gtk_menu_item_new_with_label("Ajouter des sous-titre");
        gtk_widget_show(menuVideo_items);
        gtk_menu_append(GTK_MENU (menuVideo), menuVideo_items);


        /* Now we specify that we want our newly created "menu" to be the menu for the "root menu" */
        gtk_menu_item_set_submenu(GTK_MENU_ITEM (fileLabel), menuFile);
        gtk_menu_item_set_submenu(GTK_MENU_ITEM (videoLabel), menuVideo);


        /* Create a menu-bar to hold the menus and add it to our main window*/
        menu_bar = gtk_menu_bar_new();
        gtk_container_add(GTK_CONTAINER(vbox), menu_bar);
        gtk_widget_show(menu_bar);

        /* And finally we append the menu-item to the menu-bar -- this is the "root"
         * menu-item I have been raving about =) */
        gtk_menu_bar_append(GTK_MENU_BAR (menu_bar), fileLabel);
        gtk_menu_bar_append(GTK_MENU_BAR (menu_bar), videoLabel);

    }
}

void Ui::start (Backend* back) {
	GtkWidget *button;
	GtkWidget *vbox;
    _back = back;
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
          pause_button = gtk_button_new_from_stock (GTK_STOCK_MEDIA_PAUSE);

        g_signal_connect (G_OBJECT (pause_button), "clicked", G_CALLBACK (pause_cb), NULL);

        gtk_box_pack_start (GTK_BOX (controls), pause_button, FALSE, FALSE, 2);
    }
    {
          stop_button = gtk_button_new_from_stock (GTK_STOCK_MEDIA_STOP);

        g_signal_connect (G_OBJECT (stop_button), "clicked", G_CALLBACK (stop_cb), NULL);

        gtk_box_pack_start (GTK_BOX (controls), stop_button, FALSE, FALSE, 2);
    }

    {
        play_button = gtk_button_new_from_stock (GTK_STOCK_MEDIA_PLAY);

      g_signal_connect (G_OBJECT (play_button), "clicked", G_CALLBACK (pause_cb), NULL);

      gtk_box_pack_start (GTK_BOX (controls), play_button, FALSE, FALSE, 2);
    }

    {

        button = gtk_button_new_with_label ("Hide srt");

        g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (subtitles_cb), NULL);

        gtk_box_pack_start (GTK_BOX (controls), button, FALSE, FALSE, 2);
        subtitles_button = button;
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

    {
        labelProgression = gtk_label_new("0");
        gtk_box_pack_end(GTK_BOX(controls), labelProgression, FALSE, FALSE, 2);
    }

	gtk_widget_show_all (GTK_WIDGET (window));
    gtk_widget_hide(play_button);

}

gboolean Ui::timeout (gpointer data) {
	guint64 pos;

    pos = _back->backend_query_position ();
	if (!DURATION_IS_VALID (duration))
        duration = _back->backend_query_duration ();

	if (!DURATION_IS_VALID (duration))
		return TRUE;
    std::ostringstream s1, s2;
    double currentTime = _back->backend_query_position();
    double finalTime = _back->backend_query_duration();
    currentTime /= 1000000000;
    finalTime /= 1000000000;
    s1 << std::setprecision(2) << currentTime;
    s2 << std::setprecision(2) << finalTime;
    gtk_label_set_text(GTK_LABEL(labelProgression), std::string(s1.str()+ " / "+s2.str()).c_str());
	if (pos != 0) {
		double value;
		value = (pos * (((double) 100) / duration));
		gtk_range_set_value (GTK_RANGE (scale), value);
	}

	return TRUE;
}
std::string Ui::getSrtFilename() const
{
    return _srtFilename;
}

void Ui::setSrtFilename(const std::string &srtFilename)
{
    _srtFilename = srtFilename;
}

void Ui::setFileName(std::string s) {
    _fileName = s;
}

std::string Ui::getFileName() {
    return _fileName;
}

gboolean Ui::init (gpointer data) {
    if (!_ui->getFileName().empty())
        _back->backend_play (_ui->getFileName(), _ui->getSrtFilename());

	g_timeout_add (1000, timeout, NULL);
    if(_back->subtitlesIsHiding()) {
        gtk_widget_hide(subtitles_button);
    }

	return FALSE;
}
