#pragma once

#include <cstdint>
#include <string>

namespace World
{
    class Entity {
        public:
            [[nodiscard]] virtual bool Update(float deltaTime);
            virtual void Render(float deltaTime) = 0;
            virtual void OnDestroy() {};
            virtual void OnSpawn() {};

            void Destroy() { isValid = false; }

            [[nodiscard]] virtual bool IsValid() const { return isValid; }

            void SetTagName(const std::string& newTagName) { this->tagName = newTagName; }
            [[nodiscard]] const std::string& GetTagName() const { return tagName; }
        protected:
            uint8_t isValid : 1 = true;
            std::string tagName;
    };
}
