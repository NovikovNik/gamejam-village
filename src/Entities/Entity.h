#pragma once

#include <cstdint>

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

        protected:
            uint8_t isValid : 1 = true;
    };
}
