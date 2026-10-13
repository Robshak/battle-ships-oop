#pragma once

#include "core/Entity.h"

#include <QPoint>

#include <array>

class Strategy : public Entity {
public:
    Strategy() = default;
    ~Strategy() override = default;
    virtual QPoint next(const std::array<int, 100>& shots) const = 0;
};

class HardStrategy : public Strategy {
    static bool inside(int x, int y);
    static std::array<int, 4> remainingFleet(const std::array<int, 100>& shots);
    static std::array<bool, 100> forbiddenCells(const std::array<int, 100>& shots);

public:
    HardStrategy() = default;
    ~HardStrategy() override = default;
    QPoint next(const std::array<int, 100>& shots) const override;

    QString repr() const override {
        return "HardStrategy";
    }

    QString toString() const override {
        return "Сложный — поиск и добивание";
    }

    bool operator==(const Entity& other) const override {
        return repr() == other.repr();
    }

    explicit operator bool() const override {
        return true;
    }
};

class RandomStrategy : public Strategy {
public:
    RandomStrategy() = default;
    ~RandomStrategy() override = default;
    QPoint next(const std::array<int, 100>& shots) const override;

    QString repr() const override {
        return "RandomStrategy";
    }

    QString toString() const override {
        return "Случайные выстрелы";
    }

    bool operator==(const Entity& other) const override {
        return repr() == other.repr();
    }

    explicit operator bool() const override {
        return true;
    }
};
