#include "player.h"

Player::Player(int argc, char** argv) throw(std::invalid_argument)
{
    if(argc-1 < 1) {
        throw(std::invalid_argument("You must give a video filename"));
    }
    if(argc == 1) {
        // TODO Check if there is a srt file like ogv file
    }
    _ui = new Ui((argc > 1) ? argv[1] : "", (argc > 2) ? argv[2] : "");
    _backend = new Backend(&argc, &argv);
    gtk_init(&argc, &argv);
}

void Player::open() {
    _ui->start(_backend);
    g_idle_add(Ui::init, NULL);
    gtk_main();
}

