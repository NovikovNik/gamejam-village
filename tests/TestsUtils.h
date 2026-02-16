#pragma once
#include <memory>

namespace TestsUtils {

    template <typename TData>
    class Provider {
    public:
        std::shared_ptr<TData> data = std::make_shared<TData>();
    };

}