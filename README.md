### Сборка и компиляция

1. В корне проекта
2. git submodule init
3. git submodule update --init --recursive --jobs 4
2. cmake -S . -B build
2. cmake --build build
3. ./build/game_engine
