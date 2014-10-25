#include "gstreamercommands.h"

GStreamerCommands::GStreamerCommands() {
    _pipeline = gst_pipeline_new ("audiovideo-player");
}

void GStreamerCommands::addElement(std::string name, std::string value) {
    _elements.insert(std::pair<std::string, GstElement*>(
                             name,
                             gst_element_factory_make (value.data(), name.data())));
}

void GStreamerCommands::checkAllElements() {
    std::map<std::string, GstElement*>::iterator it;
    for(it = _elements.begin() ; it != _elements.end() ; ++it) {
        if(!it->second) {
            g_printerr ("One element could not be created. Exiting.\n");
        }
    }
}

void GStreamerCommands::addAllElements() {
    std::map<std::string, GstElement*>::iterator it;
    for(it = _elements.begin() ; it != _elements.end() ; ++it) {
        gst_bin_add(GST_BIN(_pipeline), it->second);
    }

}
GstElement *GStreamerCommands::getPipeline() const
{
    return _pipeline;
}

void GStreamerCommands::setPipeline(GstElement *pipeline)
{
    _pipeline = pipeline;
}


GstElement* GStreamerCommands::getElement(std::string s) {
    return _elements.at(s);
}
