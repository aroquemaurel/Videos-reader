#ifndef CONTROLLER_H
#define CONTROLLER_H
#include <iostream>
#include <stdexcept>
#include "ui.h"
#include "backend.h"
class Player
{
public:
    Player(int argc, char** argv) throw(std::invalid_argument);
    void open();
private:
    Ui* _ui;
    Backend* _backend;
};

#endif // CONTROLLER_H
