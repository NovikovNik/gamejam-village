#pragma once

#include "Event.h"
#include <string>

class EntityCreatedEvent: public Event {
    public:
        EntityCreatedEvent(const std::string& name, const std::string& type) : name(name), type(type) {}
        const std::string& GetName() const { return name; }
        const std::string& GetType() const { return type; }
    private:
        std::string name;
        std::string type;
};

class EntityDestroyedEvent: public Event {
    public:
        EntityDestroyedEvent(const std::string& name, const std::string& type) : name(name), type(type) {}
        const std::string& GetName() const { return name; }
        const std::string& GetType() const { return type; }
    private:
        std::string name;
        std::string type;
};
