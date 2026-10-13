#pragma once

#include "core/Entity.h"

#include <QPoint>

class Ship : public Entity {
    QPoint position_;
    int length_ = 1;
    bool horizontal_ = true;
    unsigned hits_ = 0;

public:
    Ship(QPoint position = {}, int length = 1, bool horizontal = true);
    ~Ship() override = default;

    QPoint position() const {
        return position_;
    }

    int length() const {
        return length_;
    }

    bool horizontal() const {
        return horizontal_;
    }

    unsigned hits() const {
        return hits_;
    }

    bool setPlacement(QPoint position, int length, bool horizontal);
    QPoint cell(int index) const;
    bool contains(QPoint position) const;
    bool hit(QPoint position);
    int remaining() const;

    void resetHits() {
        hits_ = 0;
    }

    QString repr() const override;
    QString toString() const override;

    bool operator==(const Entity& other) const override {
        return repr() == other.repr();
    }

    explicit operator bool() const override {
        return remaining() > 0;
    }
};
