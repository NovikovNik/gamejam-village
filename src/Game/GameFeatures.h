#pragma once

enum class MovementDirection {
    FOUR_DIRECTIONS,
    EIGHT_DIRECTIONS,
};

struct GameFeatures {
    inline static std::string gameTitle = "AAAB";
    inline static bool isDebug = false;
    inline static bool isFullscreen = false;
    inline static bool isResizeble = false;
    inline static MovementDirection movementDirection = MovementDirection::EIGHT_DIRECTIONS;
};
