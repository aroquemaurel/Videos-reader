#ifndef GST_BACKEND_H
#define GST_BACKEND_H

void backend_init (int *argc, char **argv[]);
void backend_deinit (void);
void backend_set_window (gpointer window);
void backend_play (const gchar *filename);
void backend_stop (void);
void backend_seek (gint value);
void backend_seek_absolute (guint64 value);
void backend_reset (void);
void backend_pause (void);
void backend_resume (void);
guint64 backend_query_position (void);
guint64 backend_query_duration (void);

#endif /* GST_BACKEND_H */
