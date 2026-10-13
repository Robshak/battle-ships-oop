#pragma once

#include "game/Game.h"
#include "app/Session.h"
#include "ui/Widgets.h"

#include <QTimer>

class Window : public QWidget, public Entity {
    Session session_;
    Game game_;
    StrategyCombo* strategy_;
    HintCheck* hints_;
    ActionButton* start_;
    ActionButton* shuffle_;
    ActionButton* save_;
    ActionButton* load_;
    ActionButton* manual_;
    ActionButton* rotate_;
    ActionButton* undo_;
    bool manualMode_ = false;
    bool horizontal_ = true;
    FieldBoard* playerBoard_;
    FieldBoard* enemyBoard_;
    FleetProgress* playerProgress_;
    FleetProgress* enemyProgress_;
    BoardGroup* playerGroup_;
    BoardGroup* enemyGroup_;
    TitleLabel* status_;
    EventList* history_;
    QTimer* timer_;

    void updateView();
    void startGame();
    void shuffleFleet();
    void arrangeFleet();
    void rotateShip();
    void placeShip(QPoint position);
    void undoPlacement();
    void saveFleet();
    void loadFleet();
    void chooseStrategy(int index);
    void changeHints(bool enabled);
    void computerStep();
    void recordShot(const QString& who, QPoint position, Field::Result result);
    void openReports();

public:
    explicit Window(QWidget* parent = nullptr);
    ~Window() override = default;

    const Game& game() const {
        return game_;
    }

    const Session& session() const {
        return session_;
    }

    void newGame();
    void shoot(QPoint position);

    QString repr() const override {
        return "Window(" + game_.repr() + ";" + session_.repr() + ")";
    }

    QString toString() const override {
        return windowTitle() + ": " + game_.toString();
    }

    bool operator==(const Entity& other) const override {
        return repr() == other.repr();
    }

    explicit operator bool() const override {
        return isVisible();
    }

protected:
    void closeEvent(QCloseEvent* event) override;
};
