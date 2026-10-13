#pragma once

#include "core/Entity.h"

#include <QDateTime>

class Session : public Entity {
    QString login_;
    QDateTime timeIn_;
    QDateTime timeOut_;

public:
    Session() {
        begin();
    }

    ~Session() override = default;

    QString login() const {
        return login_;
    }

    QDateTime timeIn() const {
        return timeIn_;
    }

    QDateTime timeOut() const {
        return timeOut_;
    }

    static QString reportPath();
    static QString systemLogin();
    bool begin();
    bool finish(QString& error);
    QString repr() const override;
    QString toString() const override;

    bool operator==(const Entity& other) const override {
        return repr() == other.repr();
    }

    explicit operator bool() const override {
        return timeOut_.isNull();
    }
};
