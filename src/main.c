#include "game.h"

int main(int argc, char **argv) {
    Game game;
    game_init(&game);

    while(game.state != GAME_CLOSE) {
        game_update(&game);
        game_render(&game);
    }

    game_destroy(&game);
}
