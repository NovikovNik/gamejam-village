#include "EventsHandler.h"
#include "EventBus.h"

namespace {
    int32_t lastSubscriberId = 0;
}

namespace Events {
    Handler::~Handler() {
        Destroy();
    }

    Handler::Handler(Handler&& other) noexcept
        : id(other.id)
    {
        other.id = 0;
    }

    Handler& Handler::operator=(Handler&& other) noexcept {
        if (this != &other) {
            if (id != 0) EventBus::instance().Unsubscribe(id);
            id = other.id;
            other.id = 0;
        }
        return *this;
    }

    void Handler::Initialize() {
        id = ++lastSubscriberId;
    }

    void Handler::Destroy() {
        if (id != 0) {
            EventBus::instance().Unsubscribe(id);
            id = 0;
        }
    }

}
