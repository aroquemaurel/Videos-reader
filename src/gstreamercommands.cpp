#include "gstreamercommands.h"

GStreamerCommands::GStreamerCommands() {
    _pipeline = gst_pipeline_new ("audiovideo-player");
}

GStreamerCommands::~GStreamerCommands() {
//    gst_object_unref(GST_OBJECT(_pipeline));
}

void GStreamerCommands::addElement(std::string name, std::string value) {
    _elements.insert(std::pair<std::string, GstElement*>(
                             name,
                         gst_element_factory_make (value.data(), name.data())));
}

void GStreamerCommands::setElement(std::string nameElement, const std::string nameArg, const std::string valuePropertie) {
    g_object_set (G_OBJECT (getElement(nameElement)), nameArg.c_str(), valuePropertie.c_str(), NULL);
}
void GStreamerCommands::setElement(std::string nameElement, const std::string nameArg, const bool valuePropertie) {
    g_object_set (G_OBJECT (getElement(nameElement)), nameArg.c_str(), valuePropertie, NULL);
}
void GStreamerCommands::setElement(std::string nameElement, const std::string nameArg, const double valuePropertie) {
    g_object_set (G_OBJECT (getElement(nameElement)), nameArg.c_str(), valuePropertie, NULL);
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
GstElement *GStreamerCommands::getPipeline() const {
    return _pipeline;
}

void GStreamerCommands::setPipeline(GstElement *pipeline) {
    _pipeline = pipeline;
}


GstElement* GStreamerCommands::getElement(std::string s) {
    return _elements.at(s);
}
