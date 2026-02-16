### Сборка и компиляция

1. В корне проекта
2. git submodule init
3. git submodule update --init --recursive --jobs 4
4. cmake -S . -B build
5. cmake --build build
6. ./build/game_engine

Tests:
1. cmake --build build --target game_engine_tests
6. ./build/game_engine_tests
