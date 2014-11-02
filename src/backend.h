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
    void play (const std::string filename, const std::string srtfilename="");
    void quit (void);
    void pause (void);
    void reset (std::string filename, std::string srtfilename);
    void resume (void);
    void seek (gint value);
    void absoluteSeek (unsigned long value);
    void setWindow (gpointer window);
    void stop();
    unsigned long queryPosition (void);
    unsigned long queryDuration (void);

    void showSubtitles();
    void hideSubtitles();
    bool subtitlesIsHiding(void);
    void setVolume(const double volume);
    void volumeDown();
    void volumeUp();
    double getCurrentVolume() const;
    void setCurrentVolume(double getCurrentVolume);

private:
    static gboolean bus_cb(GstBus *bus, GstMessage *msg, gpointer data);
    static void on_pad_added(GstElement *element, GstPad *pad, gpointer data);

    void createBusForMessages();
    void incrustVideo();

    gpointer _window;
    GstSeekFlags _seek_flags;
    GStreamerCommands* _commands;
    bool _subtitlesIsHidding;
    double _currentVolume;
};

#endif /* GST_BACKEND_H */
