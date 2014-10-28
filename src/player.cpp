#include "player.h"

Player::Player(int argc, char** argv)
{
    _ui = new Ui((argc > 1) ? argv[1] : "", (argc > 2) ? argv[2] : "");
    _backend = new Backend(&argc, &argv);
    gtk_init(&argc, &argv);
}

void Player::open() {
    _ui->start(_backend);
    g_idle_add(Ui::init, NULL);
    gtk_main();
}

