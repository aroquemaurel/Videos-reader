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
    ~GStreamerCommands();
    GstElement *getElement(std::string s);
    void addElement(std::string name, std::string value);

    void setElement(std::string nameElement, const std::string nameArg, const std::string valuePropertie);
    void setElement(std::string nameElement, const std::string nameArg, const double valuePropertie);
    void setElement(std::string nameElement, const std::string nameArg, const bool valuePropertie);

    void checkAllElements();
    void addAllElements();
    GstElement *getPipeline() const;
    void setPipeline(GstElement *getPipeline);

private:
    std::map<std::string, GstElement*> _elements;
    GstElement *_pipeline;

};

#endif // GSTREAMERCOMMANDS_H
