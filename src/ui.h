#ifndef __UI
#define __UI
#include <string>
#include <iostream>

class Ui {
	public:
		Ui();
		void start (void);
		static void toggle_paused (void);
		static void toggle_fullscreen (void);
		static void pause_cb (GtkWidget *widget, gpointer data);
		static void reset_cb (GtkWidget *widget, gpointer data);
		static gboolean delete_event (GtkWidget *widget, GdkEvent *event, gpointer data);
		static gboolean key_press (GtkWidget *widget, GdkEventKey *event, gpointer data);
		static gboolean init (gpointer data);

		void setFileName(std::string s);
		std::string getFileName();

		static void destroy (GtkWidget *widget, gpointer data);
		static void realize_cb (GtkWidget * widget, gpointer data);
	private:
		std::string _fileName;


};

#endif


