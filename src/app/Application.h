#pragma once

#include "core/Entity.h"

#include <QApplication>

class Application : public QApplication, public Entity {
public:
    Application(int& argc, char** argv);
    ~Application() override = default;
    static int run(int argc, char** argv);

    QString repr() const override {
        return "Application(" + applicationName() + ";" + applicationVersion() + ")";
    }

    QString toString() const override {
        return applicationDisplayName();
    }

    bool operator==(const Entity& other) const override {
        return repr() == other.repr();
    }

    explicit operator bool() const override {
        return !closingDown();
    }
};
