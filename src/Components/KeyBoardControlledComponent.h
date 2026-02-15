#pragma once
#include "glm/fwd.hpp"
#include <glm/glm.hpp>

struct KeyBoardControlledComponent {
    glm::vec2 upVelocity;
    glm::vec2 rightVelocity;
    glm::vec2 downVelocity;
    glm::vec2 leftVelocity;
    KeyBoardControlledComponent(glm::vec2 upVelocity = glm::vec2(0),
        glm::vec2 rightVelocity = glm::vec2(0),
        glm::vec2 downVelocity = glm::vec2(0),
        glm::vec2 leftVelocity = glm::vec2(0)) {
            this->upVelocity = upVelocity;
            this->rightVelocity = rightVelocity;
            this->downVelocity = downVelocity;
            this->leftVelocity = leftVelocity;
    }
};
