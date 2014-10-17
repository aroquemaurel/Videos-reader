#ifndef __UI
#define __UI
#include <string>
#include <iostream>

class Ui {
	public:
		Ui();
		void start (void);
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
		static Ui* _ui;

		std::string _fileName;


};

#endif


