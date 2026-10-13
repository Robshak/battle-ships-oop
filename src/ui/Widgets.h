#pragma once

#include "model/Field.h"

#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QListWidget>
#include <QProgressBar>
#include <QPushButton>
#include <QScrollBar>

#include <functional>

class TitleLabel : public QLabel, public Entity {
public:
    explicit TitleLabel(const QString& text, QWidget* parent = nullptr);
    ~TitleLabel() override = default;

    QString repr() const override {
        return "TitleLabel(" + text() + ")";
    }

    QString toString() const override {
        return text();
    }

    bool operator==(const Entity& other) const override {
        return repr() == other.repr();
    }

    explicit operator bool() const override {
        return !text().isEmpty();
    }

protected:
    void mouseDoubleClickEvent(QMouseEvent* event) override;
};

class ActionButton : public QPushButton, public Entity {
public:
    explicit ActionButton(const QString& text, QWidget* parent = nullptr);
    ~ActionButton() override = default;

    QString repr() const override {
        return QString("ActionButton(%1,%2)").arg(text()).arg(isEnabled());
    }

    QString toString() const override {
        return text();
    }

    bool operator==(const Entity& other) const override {
        return repr() == other.repr();
    }

    explicit operator bool() const override {
        return isEnabled();
    }

protected:
    void keyPressEvent(QKeyEvent* event) override;
};

class HistoryScrollBar : public QScrollBar, public Entity {
public:
    explicit HistoryScrollBar(QWidget* parent = nullptr);
    ~HistoryScrollBar() override = default;

    QString repr() const override {
        return QString("HistoryScrollBar(%1,%2)").arg(value()).arg(maximum());
    }

    QString toString() const override {
        return QString("Позиция истории: %1 из %2").arg(value()).arg(maximum());
    }

    bool operator==(const Entity& other) const override {
        return repr() == other.repr();
    }

    explicit operator bool() const override {
        return maximum() > minimum();
    }

protected:
    void keyPressEvent(QKeyEvent* event) override;
};

class StrategyCombo : public QComboBox, public Entity {
public:
    explicit StrategyCombo(QWidget* parent = nullptr);
    ~StrategyCombo() override = default;

    QString repr() const override {
        return QString("StrategyCombo(%1)").arg(currentIndex());
    }

    QString toString() const override {
        return currentText();
    }

    bool operator==(const Entity& other) const override {
        return repr() == other.repr();
    }

    explicit operator bool() const override {
        return currentIndex() >= 0;
    }

protected:
    void wheelEvent(QWheelEvent* event) override;
};

class HintCheck : public QCheckBox, public Entity {
public:
    explicit HintCheck(QWidget* parent = nullptr);
    ~HintCheck() override = default;

    QString repr() const override {
        return QString("HintCheck(%1)").arg(isChecked());
    }

    QString toString() const override {
        return isChecked() ? "Подсказки включены" : "Подсказки выключены";
    }

    bool operator==(const Entity& other) const override {
        return repr() == other.repr();
    }

    explicit operator bool() const override {
        return isChecked();
    }

protected:
    void keyPressEvent(QKeyEvent* event) override;
};

class FleetProgress : public QProgressBar, public Entity {
public:
    explicit FleetProgress(QWidget* parent = nullptr);
    ~FleetProgress() override = default;

    QString repr() const override {
        return QString("FleetProgress(%1,%2)").arg(value()).arg(maximum());
    }

    QString toString() const override {
        return QString("Целых палуб: %1 из %2").arg(value()).arg(maximum());
    }

    bool operator==(const Entity& other) const override {
        return repr() == other.repr();
    }

    explicit operator bool() const override {
        return value() > minimum();
    }

protected:
    void mousePressEvent(QMouseEvent* event) override;
};

class EventList : public QListWidget, public Entity {
public:
    explicit EventList(QWidget* parent = nullptr);
    ~EventList() override = default;
    void addEntry(const QString& message);
    QString repr() const override;

    QString toString() const override {
        return QString("Записей в истории: %1").arg(count());
    }

    bool operator==(const Entity& other) const override {
        return repr() == other.repr();
    }

    explicit operator bool() const override {
        return count() > 0;
    }

protected:
    void keyPressEvent(QKeyEvent* event) override;
};

class BoardGroup : public QGroupBox, public Entity {
public:
    explicit BoardGroup(const QString& title, QWidget* parent = nullptr);
    ~BoardGroup() override = default;

    QString repr() const override {
        return "BoardGroup(" + title() + ")";
    }

    QString toString() const override {
        return title();
    }

    bool operator==(const Entity& other) const override {
        return repr() == other.repr();
    }

    explicit operator bool() const override {
        return !title().isEmpty();
    }

protected:
    void mouseDoubleClickEvent(QMouseEvent* event) override;
};

class FieldBoard : public QWidget, public Entity {
    const Field* field_ = nullptr;
    bool revealed_ = false;
    bool canShoot_ = false;
    bool hints_ = true;
    int placementLength_ = 0;
    bool horizontal_ = true;
    QPoint selected_{0, 0};
    std::function<void(QPoint)> onShot_;
    std::function<void(QPoint)> onPlace_;
    std::function<void()> onRotate_;

    QRect gridRect() const;
    QPoint cellAt(QPoint position) const;
    void activate();

public:
    explicit FieldBoard(QWidget* parent = nullptr);
    ~FieldBoard() override = default;

    const Field* field() const {
        return field_;
    }

    bool revealed() const {
        return revealed_;
    }

    bool canShoot() const {
        return canShoot_;
    }

    bool hints() const {
        return hints_;
    }

    int placementLength() const {
        return placementLength_;
    }

    bool horizontal() const {
        return horizontal_;
    }

    QPoint selected() const {
        return selected_;
    }

    const std::function<void(QPoint)>& onShot() const {
        return onShot_;
    }

    const std::function<void(QPoint)>& onPlace() const {
        return onPlace_;
    }

    const std::function<void()>& onRotate() const {
        return onRotate_;
    }

    void setField(const Field& field) {
        field_ = &field;
        update();
    }

    void setRevealed(bool value) {
        revealed_ = value;
        update();
    }

    void setCanShoot(bool value);
    void setPlacement(int length, bool horizontal);

    void setHints(bool value) {
        hints_ = value;
        if (!value)
            setToolTip({});
    }

    bool setSelected(QPoint position);

    void setOnShot(std::function<void(QPoint)> callback) {
        onShot_ = std::move(callback);
    }

    void setOnPlace(std::function<void(QPoint)> callback) {
        onPlace_ = std::move(callback);
    }

    void setOnRotate(std::function<void()> callback) {
        onRotate_ = std::move(callback);
    }

    QSize sizeHint() const override {
        return {360, 360};
    }

    QSize minimumSizeHint() const override {
        return {280, 280};
    }

    QString repr() const override;

    QString toString() const override {
        return field_ ? field_->toString() : "Пустое поле";
    }

    bool operator==(const Entity& other) const override {
        return repr() == other.repr();
    }

    explicit operator bool() const override {
        return field_ != nullptr;
    }

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;
};
