#pragma once

#include <glm/glm.hpp>
#include <cstdint>
#include <string>

namespace World
{
    class Entity {
        public:
            struct TagName {
                std::string name;
                std::string type;

                auto operator<=>(const TagName& other) const = default;
            };
        public:
            virtual ~Entity() = default;

            [[nodiscard]] virtual bool Update(float deltaTime);
            virtual void Render(float deltaTime) = 0;
            virtual void OnDestroy() {};
            virtual void OnSpawn(float x, float y, float w, float h);

            void Destroy() { isValid = false; }

            [[nodiscard]] virtual bool IsValid() const { return isValid; }

            virtual void SetPosition(float x, float y) { positionX = x; positionY = y; }
            [[nodiscard]] glm::vec2 GetPosition() const { return glm::vec2{ positionX, positionY }; };

            void SetTagName(const TagName& newTagName) { this->tagName = newTagName; }
            [[nodiscard]] const TagName& GetTagName() const { return tagName; }
        protected:
            uint8_t isValid : 1 = true;
            TagName tagName;

            float positionX = 0.0f;
            float positionY = 0.0f;
            float width = 0.0f;
            float height = 0.0f;
    };
}
