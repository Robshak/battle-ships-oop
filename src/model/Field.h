#pragma once

#include "model/ShipMap.h"

#include <array>

class Field : public Entity {
    ShipMap ships_;
    // 0 - еще не стреляли
    // 1 - промах
    // 2 - пападание
    // 3 - убит
    std::array<int, 100> shots_{};

public:
    enum class Result {
        Invalid,
        Repeat,
        Miss,
        Hit,
        Sunk
    };
    Field() = default;
    ~Field() override = default;
    static bool inside(QPoint position);

    const ShipMap& ships() const {
        return ships_;
    }

    const std::array<int, 100>& shots() const {
        return shots_;
    }

    const Ship* shipAt(QPoint position) const;
    int shotAt(QPoint position) const;
    bool add(const Ship& ship);
    bool canPlace(QPoint position, int length, bool horizontal) const;
    bool removeLast();
    bool randomize();
    void clear();
    bool complete() const;
    int remainingShips() const;
    int remainingDecks() const;
    Result shoot(QPoint position);
    bool save(const QString& path, QString& error) const;
    bool load(const QString& path, QString& error);
    QString repr() const override;
    QString toString() const override;

    bool operator==(const Entity& other) const override {
        return repr() == other.repr();
    }

    explicit operator bool() const override {
        return remainingShips() > 0;
    }
};
