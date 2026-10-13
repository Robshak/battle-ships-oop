#pragma once

#include "game/Strategy.h"
#include "model/Field.h"

#include <memory>

class Game : public Entity {
public:
    enum class Phase {
        Setup,
        Player,
        Computer,
        Won,
        Lost
    };

private:
    Field player_;
    Field computer_;
    std::unique_ptr<Strategy> strategy_;
    Phase phase_ = Phase::Setup;

public:
    Game();
    ~Game() override = default;

    const Field& player() const {
        return player_;
    }

    const Field& computer() const {
        return computer_;
    }

    const Strategy& strategy() const {
        return *strategy_;
    }

    Phase phase() const {
        return phase_;
    }

    bool newGame();
    bool shuffle();
    bool clearFleet();
    int nextShipLength() const;
    bool placeShip(QPoint position, bool horizontal);
    bool undoPlacement();
    bool setStrategy(int index);
    bool start();
    Field::Result shoot(QPoint position);
    std::pair<QPoint, Field::Result> computerTurn();
    bool save(const QString& path, QString& error) const;
    bool load(const QString& path, QString& error);
    QString repr() const override;
    QString toString() const override;

    bool operator==(const Entity& other) const override {
        return repr() == other.repr();
    }

    explicit operator bool() const override {
        return phase_ == Phase::Player || phase_ == Phase::Computer;
    }
};
