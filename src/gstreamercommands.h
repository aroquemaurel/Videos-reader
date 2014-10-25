#ifndef GSTREAMERCOMMANDS_H
#define GSTREAMERCOMMANDS_H
#include <iostream>
#include <map>
#include <gst/gst.h>
#include <gst/interfaces/xoverlay.h>

class GStreamerCommands
{
public:
    GStreamerCommands();

    GstElement *getElement(std::string s);
    void addElement(std::string name, std::string value);
    void checkAllElements();
    void addAllElements();
    GstElement *getPipeline() const;
    void setPipeline(GstElement *getPipeline);

private:
    std::map<std::string, GstElement*> _elements;
    GstElement *_pipeline;

};

#endif // GSTREAMERCOMMANDS_H
