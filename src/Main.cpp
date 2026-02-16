#define SDL_MAIN_HANDLED
#include <Game/Game.h>
#include <FileSystem/FileSystem.h>

int main(int argc, char* argv[]) {
    Game game;

    FileSystemManager::SetExecutableDir(argv[0]);

    // Game skeleton
    game.Initialize();
    game.Run();
    game.Destroy();
    // End Game skeleton

    return 0;
}
