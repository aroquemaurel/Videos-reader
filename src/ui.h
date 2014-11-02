#ifndef __UI
#define __UI

#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <gdk/gdkkeysyms.h>

#include <iostream>
#include <iomanip>
#include <sstream>

#include "backend.h"
class Ui {
	public:
		Ui();
        Ui(std::string filepath, std::string srtfilename="");

        void start (Backend *back);
		std::string getFileName();
        static gboolean init (gpointer data);
        std::string getSrtFilename() const;

private:
        static bool delete_event (GtkWidget *widget, GdkEvent *event, gpointer data);
        static bool key_press (GtkWidget *widget, GdkEventKey *event, gpointer data);
        static bool mousse_press(GtkWidget *widget, GdkEvent *event, gpointer data);
        static gboolean timeout(gpointer data);
        static void destroy (GtkWidget *widget, gpointer data);

        static void subtitles_cb(GtkWidget *widget, gpointer data);
        static void seek_cb(GtkRange *range, GtkScrollType scroll, gdouble value, gpointer data);
        static void pause_cb (GtkWidget *widget, gpointer data);
        static void reset_cb (GtkWidget *widget, gpointer data);
        static void realize_cb (GtkWidget * widget, gpointer data);
        static void stop_cb(GtkWidget *widget, gpointer data);
        static void volume_cb(GtkRange *range, GtkScrollType scroll, gdouble value, gpointer data);
        static void volume_up();
        static void volume_down();
        void toggle_paused (void);
        void toggle_fullscreen (void);


        inline static bool durationIsValid(const unsigned int x);
        inline static std::string time2String(const unsigned long time);


        std::string _fileName;
        std::string _srtFilename;

        static Ui* _ui;
        static Backend* _back;
        static int _lastVoume;
        static GtkWidget* _video_output;
        static GtkWidget* _pause_button;
        static GtkWidget* _play_button;
        static GtkWidget* _scale;
        static GtkWidget* _volume;
        static GtkWindow* _window;
        static GtkWidget* _controls;
        static GtkWidget* _subtitles_button;
        static GtkWidget* _stop_button;
        static GtkWidget* _labelProgression;

        static unsigned long _duration;
};


#endif


