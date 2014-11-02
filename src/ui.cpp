#include "ui.h"
#include "backend.h"
#include "player.h"


unsigned long Ui::_duration = 0;
GtkWidget* Ui::_video_output = 0;
GtkWidget* Ui::_pause_button = 0;
GtkWidget* Ui::_play_button = 0;
GtkWidget* Ui::_scale = 0;
GtkWidget* Ui::_volume = 0;
GtkWindow* Ui::_window = 0;
GtkWidget* Ui::_controls = 0;
GtkWidget* Ui::_subtitles_button = 0;
GtkWidget* Ui::_stop_button = 0;
GtkWidget* Ui::_labelProgression = 0;

Ui* Ui::_ui = 0;
Backend* Ui::_back = 0;

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
        _back->resume ();
        gtk_widget_hide(_play_button);
        gtk_widget_show(_pause_button);
        paused = false;
	} else {
        _back->pause ();
        gtk_widget_hide(_pause_button);
        gtk_widget_show(_play_button);
        paused = true;
	}
}

void Ui::toggle_fullscreen (void) {
    if (gdk_window_get_state (GTK_WIDGET (_window)->window) == GDK_WINDOW_STATE_FULLSCREEN) {
        gtk_window_unfullscreen (_window);
        gtk_widget_show (_controls);
	} else {
        gtk_window_fullscreen (_window);
        gtk_widget_hide (_controls);
	}
}

void Ui::pause_cb (GtkWidget *widget, gpointer data) {
	_ui->toggle_paused();
}

void Ui::stop_cb(GtkWidget *widget, gpointer data) {
    _back->stop();
    gtk_range_set_value (GTK_RANGE (_scale), 0);
}

void Ui::reset_cb (GtkWidget *widget, gpointer data) {
    _back->reset (_ui->getFileName(), _ui->getSrtFilename());
}

bool Ui::delete_event (GtkWidget *widget, GdkEvent *event, gpointer data) {
    _back->stop ();
    return false;
}

void Ui::destroy (GtkWidget *widget, gpointer data) {
    gtk_main_quit ();
}

bool Ui::key_press (GtkWidget *widget, GdkEventKey *event, gpointer data) {
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
            _back->reset (_ui->getFileName(), _ui->getSrtFilename());
			break;
		case GDK_Right:
            _back->seek (10);
			break;
		case GDK_Left:
            _back->seek (-10);
			break;
		case GDK_Escape:
		case GDK_Q:
		case GDK_q:
			gtk_main_quit ();
			break;
        case GDK_S:
        case GDK_s:
            _ui->subtitles_cb(NULL, 0);
        break;
        case GDK_Up:
            _ui->volume_up();
            break;
    case GDK_Down:
            _ui->volume_down();
            break;
    case GDK_BackSpace:
        _ui->stop_cb(NULL, 0);
		default:
			break;
	}

    return true;
}
void Ui::volume_cb (GtkRange *range, GtkScrollType scroll, gdouble value, gpointer data) {
    _back->setVolume(value/100);
}
void Ui::volume_up () {
    _back->volumeUp();
}
void Ui::volume_down() {
    _back->volumeDown();
}

void Ui::seek_cb (GtkRange *range, GtkScrollType scroll, gdouble value, gpointer data)
{
	guint64 to_seek;

    if (!durationIsValid (_duration))
        _duration = _back->queryDuration ();

    if (!durationIsValid (_duration))
		return;

    to_seek = (value / 100) * _duration;

    _back->absoluteSeek (to_seek);
}

void Ui::realize_cb (GtkWidget * widget, gpointer data) {
    GdkWindow *window = gtk_widget_get_window (_video_output);

	gdk_window_ensure_native (window);
    _back->setWindow (GINT_TO_POINTER (GDK_WINDOW_XID (window)));
}

void Ui::subtitles_cb(GtkWidget * widget, gpointer data) {
    if(_back->subtitlesIsHiding()) {
        _back->showSubtitles();
        gtk_button_set_label(GTK_BUTTON(_subtitles_button), "Hide srt");
    } else {
        _back->hideSubtitles();
        gtk_button_set_label(GTK_BUTTON(_subtitles_button), "Show srt");
    }
}

void Ui::start (Backend* back) {
	GtkWidget *button;
	GtkWidget *vbox;
    _back = back;
    _window = GTK_WINDOW (gtk_window_new (GTK_WINDOW_TOPLEVEL));

    g_signal_connect (G_OBJECT (_window), "delete_event", G_CALLBACK (delete_event), NULL);
    g_signal_connect (G_OBJECT (_window), "destroy", G_CALLBACK (Ui::destroy), NULL);
    g_signal_connect (G_OBJECT (_window), "key-press-event", G_CALLBACK (key_press), NULL);
    gtk_container_set_border_width (GTK_CONTAINER (_window), 0);

    vbox = gtk_vbox_new (false, 0);

    gtk_container_add (GTK_CONTAINER (_window), vbox);

    _controls = gtk_hbox_new (false, 0);

    gtk_box_pack_end (GTK_BOX (vbox), _controls, false, false, 2);

	{
		GdkColor color;

		gdk_color_parse ("black", &color);
        _video_output = gtk_drawing_area_new ();
        gtk_widget_modify_bg (GTK_WIDGET (_video_output), GTK_STATE_NORMAL, &color);
        gtk_widget_set_double_buffered (_video_output, false);

        gtk_box_pack_start (GTK_BOX (vbox), _video_output, true, true, 0);

        gtk_widget_set_size_request (_video_output, 0x200, 0x100);

        g_signal_connect (_video_output, "realize", G_CALLBACK (realize_cb), NULL);

	}

	{
          _pause_button = gtk_button_new_from_stock (GTK_STOCK_MEDIA_PAUSE);

        g_signal_connect (G_OBJECT (_pause_button), "clicked", G_CALLBACK (pause_cb), NULL);

        gtk_box_pack_start (GTK_BOX (_controls), _pause_button, false, false, 2);
    }
    {
          _stop_button = gtk_button_new_from_stock (GTK_STOCK_MEDIA_STOP);

        g_signal_connect (G_OBJECT (_stop_button), "clicked", G_CALLBACK (stop_cb), NULL);

        gtk_box_pack_start (GTK_BOX (_controls), _stop_button, false, false, 2);
    }

    {
        _play_button = gtk_button_new_from_stock (GTK_STOCK_MEDIA_PLAY);

      g_signal_connect (G_OBJECT (_play_button), "clicked", G_CALLBACK (pause_cb), NULL);

      gtk_box_pack_start (GTK_BOX (_controls), _play_button, false, false, 2);
    }

    {

        button = gtk_button_new_with_label ("Hide srt");

        g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (subtitles_cb), NULL);

        gtk_box_pack_start (GTK_BOX (_controls), button, false, false, 2);
        _subtitles_button = button;
    }

	{
        button = gtk_button_new_with_label ("Reset");
		g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (reset_cb), NULL);

        gtk_box_pack_start (GTK_BOX (_controls), button, false, false, 2);
	}

    {
		GtkObject *adjustment;
		adjustment = gtk_adjustment_new (0, 0, 101, 1, 5, 1);
        _scale = gtk_hscale_new (GTK_ADJUSTMENT (adjustment));

        gtk_box_pack_end (GTK_BOX (_controls), _scale, true, true, 2);

        g_signal_connect (G_OBJECT (_scale), "change-value", G_CALLBACK (seek_cb), NULL);
	}
    {
        GtkObject *adjustment;
        _volume = gtk_volume_button_new ();
        adjustment = gtk_adjustment_new (0, 0, 200, 1, 5, 1);
        gtk_scale_button_set_adjustment(GTK_SCALE_BUTTON(_volume), GTK_ADJUSTMENT(adjustment));
        gtk_scale_button_set_value(GTK_SCALE_BUTTON(_volume), 50);
        gtk_box_pack_end (GTK_BOX (_controls), _volume, false, false, 2);

        g_signal_connect (G_OBJECT (_volume), "value-changed", G_CALLBACK (volume_cb), NULL);
    }

    {
        _labelProgression = gtk_label_new("00:00:00 / 00:00:00");
        gtk_box_pack_end(GTK_BOX(_controls), _labelProgression, false, false, 2);
    }

    gtk_widget_show_all (GTK_WIDGET (_window));
    gtk_widget_hide(_play_button);

}
inline std::string Ui::time2String(const unsigned long time) {
    char buff[512];
    std::string buffString;
    std::string ret;
    sprintf(buff, "%" GST_TIME_FORMAT "\n", GST_TIME_ARGS(time));
    buffString = std::string(buff);
    ret = buffString.substr(0, buffString.find("."));

    return ret;
}

gboolean Ui::timeout (gpointer data) {
    unsigned long pos;

    pos = _back->queryPosition ();
    if (!durationIsValid (_duration))
        _duration = _back->queryDuration ();

    if (!durationIsValid (_duration))
        return true;

    gtk_label_set_text(GTK_LABEL(_labelProgression), std::string(time2String(_back->queryPosition())+ " / "+
                                                                time2String(_back->queryDuration())).c_str());
	if (pos != 0) {
		double value;
        value = (pos * (((double) 100) / _duration));
        gtk_range_set_value (GTK_RANGE (_scale), value);
	}

    return true;
}

bool Ui::durationIsValid(const unsigned int x) {
    return (x != 0 && x != (const unsigned int) -1);
}
std::string Ui::getSrtFilename() const
{
    return _srtFilename;
}

std::string Ui::getFileName() {
    return _fileName;
}

gboolean Ui::init (gpointer data) {
    if (!_ui->getFileName().empty()) {
        gtk_window_set_title(_window, _ui->getFileName().c_str());
        _back->play (_ui->getFileName(), _ui->getSrtFilename());
    }

    g_timeout_add (300, timeout, NULL);
    if(_back->subtitlesIsHiding()) {
        gtk_widget_hide(_subtitles_button);
    }

    return false;
}
