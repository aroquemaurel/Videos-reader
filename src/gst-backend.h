#ifndef GST_BACKEND_H
#define GST_BACKEND_H

#include <gst/gst.h>
#include <gst/interfaces/xoverlay.h>
#include <iostream>

#include "gstreamercommands.h"

class Backend {
public:
    Backend(int *argc, char **argv[]);
    ~Backend();
    void backend_play (const std::string filename, const std::string srtfilename="");
    void backend_stop (void);
    void backend_pause (void);
    void backend_reset (void);
    void backend_resume (void);
    void backend_seek (gint value);
    void backend_seek_absolute (guint64 value);
    void backend_set_window (gpointer window);

    guint64 backend_query_position (void);
    guint64 backend_query_duration (void);

    void showSubtitles();
    void hideSubtitles();
    bool subtitlesIsHiding();
private:
    gpointer _window;
    GstSeekFlags _seek_flags;
    GStreamerCommands* _commands;

    void createBusForMessages();
    void incrustVideo();

    bool _subtitlesIsHidding;
};

#endif /* GST_BACKEND_H */
