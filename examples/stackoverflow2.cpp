#include <gst/interfaces/xoverlay.h>
#include <gtk/gtk.h>
#include <gst/gst.h>
#include <gdk/gdkx.h>

GstElement *play;
GtkAdjustment *progress;
GtkWidget *mainwindow, *drawingarea;

class TopWin
{
public:
  TopWin();
  ~TopWin();
  int Initialize(int argc, char *argv[]);
  int Execute();
  static void FileChooser(GtkButton *button, GtkWindow *mainwindow);
  static int Play(gchar *addr);
  static gboolean print_position(GstElement *element);
private:
};

TopWin::TopWin() {
}

TopWin::~TopWin() {
}

gboolean TopWin::print_position(GstElement *play) {
  GstFormat fmt = GST_FORMAT_TIME;
  gint64 pos, len;

  if (gst_element_query_position(play, &fmt, &pos) && gst_element_query_duration(play, &fmt, &len)) {
    g_print("Time: %" GST_TIME_FORMAT " / %" GST_TIME_FORMAT "\r", GST_TIME_ARGS(pos), GST_TIME_ARGS(len));

    gtk_adjustment_set_value(GTK_ADJUSTMENT(progress), (pos*100)/len);
  }

  return TRUE;
}

int TopWin::Play(gchar *addr) {
  GMainLoop *loop;
  GstBus *bus;

  loop = g_main_loop_new(NULL, FALSE);

  play = gst_element_factory_make("playbin", "play");
  g_object_set(G_OBJECT(play), "uri", addr, NULL);

  bus = gst_pipeline_get_bus(GST_PIPELINE(play));
  gst_object_unref(bus);

  GstElement* x_overlay = gst_element_factory_make("xvimagesink", "videosink");

  g_object_set(G_OBJECT(play), "video-sink", x_overlay, NULL);

  gst_x_overlay_set_window_handle(GST_X_OVERLAY(x_overlay), GDK_WINDOW_XID(drawingarea->window));

  gst_element_set_state(play, GST_STATE_NULL);

  g_timeout_add(1000, (GSourceFunc) print_position, play);

  gtk_adjustment_set_value(GTK_ADJUSTMENT(progress), 0);

  gst_element_set_state(play, GST_STATE_PLAYING);

  g_main_loop_run(loop);

  gst_element_set_state(play, GST_STATE_NULL);
  gst_object_unref(GST_OBJECT(play));

  gtk_widget_show_all(mainwindow);
  gtk_widget_realize(drawingarea);

  return 0;
}

void TopWin::FileChooser(GtkButton *button, GtkWindow *mainwindow) {
  GtkWidget *filechooser;
  gchar *uri;

  filechooser = gtk_file_chooser_dialog_new("Open File...", mainwindow, GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OK, GTK_RESPONSE_OK, NULL);

  gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(filechooser), FALSE);

  gint response = gtk_dialog_run(GTK_DIALOG(filechooser));

  if (response == GTK_RESPONSE_OK) {
    uri = gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(filechooser));
    gtk_widget_destroy(filechooser);
    Play(uri);
    g_free(uri);
  }
  else if (response == GTK_RESPONSE_CANCEL) {
    gtk_widget_destroy(filechooser);
  }
}

int TopWin::Initialize(int argc, char *argv[]) {
  GtkWidget *playbutton, *openbutton, *volumebutton;
  GtkWidget *prefbutton, *notebook;
  GtkWidget *vbox, *hbox;
  GtkWidget *entry, *hscale;

  gtk_init(&argc, &argv);
  gst_init(&argc, &argv);

  mainwindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_container_set_border_width(GTK_CONTAINER(mainwindow), 0);
  g_signal_connect(G_OBJECT(mainwindow), "destroy", G_CALLBACK(gtk_main_quit), NULL);

  playbutton = gtk_button_new();
  gtk_button_set_image(GTK_BUTTON(playbutton), gtk_image_new_from_stock(GTK_STOCK_MEDIA_PLAY, GTK_ICON_SIZE_SMALL_TOOLBAR));

  openbutton = gtk_button_new();
  gtk_button_set_image(GTK_BUTTON(openbutton), gtk_image_new_from_stock(GTK_STOCK_OPEN, GTK_ICON_SIZE_SMALL_TOOLBAR));
  g_signal_connect(G_OBJECT(openbutton), "clicked", G_CALLBACK(TopWin::FileChooser), (gpointer) mainwindow);

  volumebutton = gtk_button_new();
  gtk_button_set_image(GTK_BUTTON(volumebutton), gtk_image_new_from_file("volume.png"));

  prefbutton = gtk_button_new();
  gtk_button_set_image(GTK_BUTTON(prefbutton), gtk_image_new_from_stock(GTK_STOCK_EXECUTE, GTK_ICON_SIZE_SMALL_TOOLBAR));

  entry = gtk_entry_new();

  progress = GTK_ADJUSTMENT(gtk_adjustment_new(0.00, 0.00, 100.00, 1.00, 0.00, 0.00));

  hscale = gtk_hscale_new(progress);
  gtk_scale_set_draw_value(GTK_SCALE(hscale), FALSE);
  gtk_widget_set_size_request(hscale, 200, NULL);

  hbox = gtk_hbox_new(FALSE, 0);
  drawingarea = gtk_drawing_area_new();
  vbox = gtk_vbox_new(FALSE, 0);

  gtk_box_pack_start(GTK_BOX(hbox), openbutton, FALSE, FALSE, 2);
  gtk_box_pack_start(GTK_BOX(hbox), playbutton, FALSE, FALSE, 2);

  gtk_box_pack_start(GTK_BOX(hbox), hscale, FALSE, FALSE, 2);
  gtk_box_pack_start(GTK_BOX(hbox), volumebutton, FALSE, FALSE, 2);
  gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(hbox), prefbutton, FALSE, FALSE, 2);

  gtk_button_set_relief(GTK_BUTTON(playbutton), GTK_RELIEF_NONE);
  gtk_button_set_relief(GTK_BUTTON(openbutton), GTK_RELIEF_NONE);
  gtk_button_set_relief(GTK_BUTTON(volumebutton), GTK_RELIEF_NONE);
  gtk_button_set_relief(GTK_BUTTON(prefbutton), GTK_RELIEF_NONE);

  gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), drawingarea, FALSE, FALSE, 0);

  gtk_container_add(GTK_CONTAINER(mainwindow), vbox);

  gtk_widget_show_all(mainwindow);

  gtk_widget_realize(drawingarea);

  return 0;
}

int TopWin::Execute() {
  gtk_main();

  return 0;
}

int main(int argc, char *argv[]) 
{
  int result = 0;
  TopWin* topwin = new TopWin();

  if (0 == topwin->Initialize(argc, argv)) {
    result = topwin->Execute();
  }

  delete topwin;

  return result;
}
