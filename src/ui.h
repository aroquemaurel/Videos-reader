#ifndef __UI
#define __UI

#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <gdk/gdkkeysyms.h>

#include <iostream>

#include "gst-backend.h"
class Ui {
	public:
		Ui();
        Ui(std::string filepath, std::string srtfilename="");

        void start (Backend *back);
		void toggle_paused (void);
		void toggle_fullscreen (void);

		void setFileName(std::string s);
		std::string getFileName();


		static gboolean init (gpointer data);

        static void setUi(Ui* ui);

        void createMenus(GtkWidget *vbox, GtkWidget *menuVideo, GtkWidget *menuVideo_items, GtkWidget *fileLabel, GtkWidget *videoLabel, GtkWidget *menu_bar, GtkWidget *menuFile_items, GtkWidget *menuFile);
        void createMenus(GtkWidget *vbox);
        std::string getSrtFilename() const;
        void setSrtFilename(const std::string &getSrtFilename);


        static void stop_cb(GtkWidget *widget, gpointer data);
private:
        static gboolean delete_event (GtkWidget *widget, GdkEvent *event, gpointer data);
		static void destroy (GtkWidget *widget, gpointer data);
		static void realize_cb (GtkWidget * widget, gpointer data);
		static void pause_cb (GtkWidget *widget, gpointer data);
		static void reset_cb (GtkWidget *widget, gpointer data);
		static gboolean key_press (GtkWidget *widget, GdkEventKey *event, gpointer data);
        static void subtitles_cb(GtkWidget *widget, gpointer data);
        static void seek_cb(GtkRange *range, GtkScrollType scroll, gdouble value, gpointer data);
        static gboolean timeout(gpointer data);

        static Ui* _ui;
        static Backend* _back;

        static GtkWidget *menu_bar;

		std::string _fileName;
        std::string _srtFilename;

};


#endif


