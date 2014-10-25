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
        Ui(std::string filepath);

        void start (Backend *back);
		void toggle_paused (void);
		void toggle_fullscreen (void);

		void setFileName(std::string s);
		std::string getFileName();


		static gboolean init (gpointer data);

        static void setUi(Ui* ui);

private:
		static gboolean delete_event (GtkWidget *widget, GdkEvent *event, gpointer data);
		static void destroy (GtkWidget *widget, gpointer data);
		static void realize_cb (GtkWidget * widget, gpointer data);
		static void pause_cb (GtkWidget *widget, gpointer data);
		static void reset_cb (GtkWidget *widget, gpointer data);
		static gboolean key_press (GtkWidget *widget, GdkEventKey *event, gpointer data);

        static void seek_cb(GtkRange *range, GtkScrollType scroll, gdouble value, gpointer data);
        static gboolean timeout(gpointer data);

        static Ui* _ui;
        static Backend* _back;

		std::string _fileName;
};

#endif


