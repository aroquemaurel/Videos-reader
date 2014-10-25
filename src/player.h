#ifndef CONTROLLER_H
#define CONTROLLER_H
#include <iostream>

#include "ui.h"
#include "gst-backend.h"
class Player
{
public:
    Player(int argc, char** argv);
    void open();
private:
    Ui* _ui;
    Backend* _backend;
};

#endif // CONTROLLER_H
