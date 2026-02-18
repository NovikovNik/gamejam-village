#pragma once
#include <cstdint>

namespace Events {

    class Handler {
    public:
        Handler() = default;
        ~Handler();

        Handler(const Handler&) = delete;
        Handler& operator=(const Handler&) = delete;
        Handler(Handler&& other) noexcept;
        Handler& operator=(Handler&& other) noexcept;

        void Initialize();
        void Destroy();
        [[nodiscard]] int32_t GetId() const { return id; }

    private:
        int32_t id{};
    };

}
