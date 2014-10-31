#include "player.h"
#include <stdexcept>
int main (int argc, char *argv[]) {
    try {
        Player p = Player(argc, argv);
        p.open();
    } catch(std::invalid_argument) {
        printf("Usage: ./lecteurvideo <Ogg/Vorbis filename> [<srt filename>]");
    }

    return 0;
}

