#pragma once

#include "model/Ship.h"

#include <utility>
#include <vector>

class ShipMap : public Entity {
    std::vector<std::pair<QPoint, Ship>> entries_;

public:
    ShipMap() = default;
    ~ShipMap() override = default;

    const std::vector<std::pair<QPoint, Ship>>& entries() const {
        return entries_;
    }

    int size() const {
        return static_cast<int>(entries_.size());
    }

    const Ship* find(QPoint key) const {
        for (const auto& item : entries_)
            if (item.first == key)
                return &item.second;
        return nullptr;
    }

    bool insert(const Ship& ship) {
        if (find(ship.position()))
            return false;
        entries_.push_back({ship.position(), ship});
        return true;
    }

    bool hit(QPoint key, QPoint position) {
        for (auto& item : entries_)
            if (item.first == key)
                return item.second.hit(position);
        return false;
    }

    bool removeLast() {
        if (entries_.empty())
            return false;
        entries_.pop_back();
        return true;
    }

    void clear() {
        entries_.clear();
    }

    QString repr() const override {
        QString value = "ShipMap[";
        for (int y = 0; y < 10; ++y)
            for (int x = 0; x < 10; ++x)
                if (const Ship* ship = find({x, y}))
                    value += ship->repr() + ";";
        return value + "]";
    }

    QString toString() const override {
        return QString("Кораблей в коллекции: %1").arg(size());
    }

    bool operator==(const Entity& other) const override {
        return repr() == other.repr();
    }

    explicit operator bool() const override {
        return !entries_.empty();
    }
};
