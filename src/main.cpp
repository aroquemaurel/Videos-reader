#include "player.h"

int main (int argc, char *argv[]) {
    Player p = Player(argc, argv);
    p.open();

    return 0;
}

