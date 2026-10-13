#pragma once

#include <QString>

class Entity {
public:
    Entity() = default;
    virtual ~Entity() = default;
    virtual QString repr() const = 0;
    virtual QString toString() const = 0;
    virtual bool operator==(const Entity& other) const = 0;
    virtual explicit operator bool() const = 0;

    bool operator!=(const Entity& other) const {
        return !(*this == other);
    }

    bool operator<(const Entity& other) const {
        return repr() < other.repr();
    }
};
