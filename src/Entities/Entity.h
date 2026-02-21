#pragma once

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
