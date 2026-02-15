#define SDL_MAIN_HANDLED
#include "./Game/Game.h"

int main(int argc, char* argv[]) {
    Game game;

    // Game skeleton
    game.Initialize();
    game.Run();
    game.Destroy();
    // End Game skeleton

    return 0;
}
