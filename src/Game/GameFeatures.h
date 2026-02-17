#pragma once

enum class MovementDirection {
    FOUR_DIRECTIONS,
    EIGHT_DIRECTIONS,
};

struct GameFeatures {
    inline static std::string windowTitle = "Test Game";
    inline static bool isDebug = true;
    inline static bool isFullscreen = false;
    inline static bool isResizeble = false;
    inline static MovementDirection movementDirection = MovementDirection::FOUR_DIRECTIONS;
};
