#include "ui/Window.h"

#include <QCloseEvent>
#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QUrl>
#include <QVBoxLayout>

Window::Window(QWidget* parent) : QWidget(parent) {
    setWindowTitle("Морской бой");
    resize(880, 820);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(20, 16, 20, 16);
    root->setSpacing(10);

    auto* title = new TitleLabel("МОРСКОЙ БОЙ", this);
    title->setStyleSheet("font-size: 24px; font-weight: bold; color: #183d61;");
    root->addWidget(title);
    root->addWidget(new TitleLabel(
        "Потопите флот противника. Попадание даёт ещё один выстрел, промах передаёт ход.", this));

    auto* settings = new QHBoxLayout;
    strategy_ = new StrategyCombo(this);
    hints_ = new HintCheck(this);
    settings->addWidget(new TitleLabel("Стрельба компьютера", this));
    settings->addWidget(strategy_, 1);
    settings->addWidget(hints_);
    root->addLayout(settings);

    auto* actions = new QHBoxLayout;
    auto* reset = new ActionButton("Новая игра", this);
    shuffle_ = new ActionButton("Случайный флот", this);
    save_ = new ActionButton("Сохранить флот", this);
    load_ = new ActionButton("Загрузить флот", this);
    start_ = new ActionButton("Начать бой", this);
    start_->setStyleSheet("QPushButton { background: #225c88; color: white; font-weight: bold; } "
                          "QPushButton:disabled { background: #dce4ea; color: #7d8995; }");
    for (auto* button : {reset, shuffle_, save_, load_, start_})
        actions->addWidget(button);
    root->addLayout(actions);

    auto* placement = new QHBoxLayout;
    manual_ = new ActionButton("Расставить вручную", this);
    manual_->setToolTip("Очистить своё поле и расставить 10 кораблей, начиная с самого большого");
    rotate_ = new ActionButton("Повернуть ↔ / ↕", this);
    rotate_->setToolTip("Клавиша R или правая кнопка мыши на поле");
    undo_ = new ActionButton("Отменить корабль", this);
    undo_->setToolTip("Убрать последний поставленный корабль");
    for (auto* button : {manual_, rotate_, undo_})
        placement->addWidget(button);
    root->addLayout(placement);

    auto* fields = new QHBoxLayout;
    playerGroup_ = new BoardGroup("Ваш флот", this);
    enemyGroup_ = new BoardGroup("Флот противника", this);

    playerBoard_ = new FieldBoard(playerGroup_);
    enemyBoard_ = new FieldBoard(enemyGroup_);
    playerBoard_->setField(game_.player());
    enemyBoard_->setField(game_.computer());

    playerProgress_ = new FleetProgress(playerGroup_);
    enemyProgress_ = new FleetProgress(enemyGroup_);

    auto* left = new QVBoxLayout(playerGroup_);
    left->addWidget(playerBoard_, 1);
    left->addWidget(playerProgress_);

    auto* right = new QVBoxLayout(enemyGroup_);
    right->addWidget(enemyBoard_, 1);
    right->addWidget(enemyProgress_);

    fields->addWidget(playerGroup_, 1);
    fields->addWidget(enemyGroup_, 1);
    root->addLayout(fields, 1);
    root->addWidget(new TitleLabel(
        "Синие клетки — корабли • точка — промах • крест — попадание • красный корабль — потоплен",
        this));

    status_ = new TitleLabel({}, this);
    status_->setStyleSheet("font-weight: bold; color: #183d61; font-size: 15px;");
    root->addWidget(status_);
    root->addWidget(new TitleLabel(
        "Стрелки — выбор клетки, Enter / пробел — поставить или выстрелить. R / правая кнопка — поворот.",
        this));

    history_ = new EventList(this);
    root->addWidget(history_);

    auto* reports = new ActionButton("Открыть папку журнала", this);
    reports->setToolTip(Session::reportPath());
    root->addWidget(reports, 0, Qt::AlignRight);

    timer_ = new QTimer(this);
    timer_->setSingleShot(true);
    timer_->setInterval(420);

    connect(timer_, &QTimer::timeout, this, &Window::computerStep);
    connect(reset, &QPushButton::clicked, this, &Window::newGame);
    connect(shuffle_, &QPushButton::clicked, this, &Window::shuffleFleet);
    connect(manual_, &QPushButton::clicked, this, &Window::arrangeFleet);
    connect(rotate_, &QPushButton::clicked, this, &Window::rotateShip);
    connect(undo_, &QPushButton::clicked, this, &Window::undoPlacement);
    connect(start_, &QPushButton::clicked, this, &Window::startGame);
    connect(save_, &QPushButton::clicked, this, &Window::saveFleet);
    connect(load_, &QPushButton::clicked, this, &Window::loadFleet);
    connect(reports, &QPushButton::clicked, this, &Window::openReports);
    connect(strategy_, &QComboBox::currentIndexChanged, this, &Window::chooseStrategy);
    connect(hints_, &QCheckBox::toggled, this, &Window::changeHints);

    enemyBoard_->setOnShot([this](QPoint position) {
        shoot(position);
    });
    playerBoard_->setOnPlace([this](QPoint position) {
        placeShip(position);
    });
    playerBoard_->setOnRotate([this] {
        rotateShip();
    });

    history_->addEntry(
        "Флот готов: 10 кораблей, 20 палуб. Можно изменить или загрузить расстановку.");
    updateView();
}

void Window::updateView() {
    bool setup = game_.phase() == Game::Phase::Setup;
    bool finished = game_.phase() == Game::Phase::Won || game_.phase() == Game::Phase::Lost;

    for (auto* button : {shuffle_, load_, manual_})
        button->setEnabled(setup);
    start_->setEnabled(setup && game_.player().complete());
    save_->setEnabled(setup && game_.player().complete());

    int length = setup && manualMode_ ? game_.nextShipLength() : 0;
    rotate_->setEnabled(length > 1);
    undo_->setEnabled(setup && manualMode_ && game_.player().ships().size() > 0);
    playerBoard_->setPlacement(length, horizontal_);

    strategy_->setEnabled(setup);
    playerBoard_->setRevealed(true);
    enemyBoard_->setRevealed(finished);
    enemyBoard_->setCanShoot(game_.phase() == Game::Phase::Player);

    playerProgress_->setValue(game_.player().remainingDecks());
    enemyProgress_->setValue(game_.computer().remainingDecks());
    playerGroup_->setTitle(QString("Ваш флот · %1 кораблей").arg(game_.player().remainingShips()));
    enemyGroup_->setTitle(
        QString("Противник · %1 кораблей").arg(game_.computer().remainingShips()));

    if (length)
        status_->setText(
            QString("Корабль %1 из 10: %2 палуб. %3. Зелёное — можно, красное — нельзя.")
                .arg(game_.player().ships().size() + 1)
                .arg(length)
                .arg(horizontal_ ? "Горизонтально" : "Вертикально"));
    else
        status_->setText(game_.toString());

    playerBoard_->update();
    enemyBoard_->update();

    if (game_.phase() == Game::Phase::Computer)
        timer_->start();
}

void Window::newGame() {
    timer_->stop();
    if (!game_.newGame()) {
        QMessageBox::warning(this, "Расстановка", "Не удалось расставить флот. Попробуйте снова.");
        if (game_.phase() == Game::Phase::Computer)
            timer_->start();
        return;
    }

    manualMode_ = false;
    history_->clear();
    history_->addEntry("Новая игра. Проверьте расстановку и нажмите «Начать бой».");
    updateView();
}

void Window::startGame() {
    game_.setStrategy(strategy_->currentIndex());
    if (!game_.start()) {
        QMessageBox::warning(this, "Начало боя", "Сначала подготовьте полный флот.");
        return;
    }

    manualMode_ = false;
    history_->addEntry("Бой начался. Вы стреляете первым.");
    updateView();
    enemyBoard_->setFocus();
}

void Window::shuffleFleet() {
    if (!game_.shuffle())
        QMessageBox::warning(this, "Расстановка", "Не удалось переставить корабли.");
    else {
        manualMode_ = false;
        history_->addEntry("Корабли переставлены случайным образом.");
    }
    updateView();
}

void Window::arrangeFleet() {
    if (!game_.clearFleet())
        return;

    manualMode_ = true;
    horizontal_ = true;
    playerBoard_->setSelected({0, 0});
    history_->addEntry(
        "Расставьте флот на левом поле. Корабли не должны касаться даже по диагонали.");
    updateView();
    playerBoard_->setFocus();
}

void Window::rotateShip() {
    if (!manualMode_ || game_.phase() != Game::Phase::Setup)
        return;

    horizontal_ = !horizontal_;
    updateView();
}

void Window::placeShip(QPoint position) {
    if (!manualMode_)
        return;
    if (!game_.placeShip(position, horizontal_)) {
        history_->addEntry(
            "Здесь нельзя поставить корабль: оставьте одну свободную клетку между кораблями.");
        return;
    }

    if (game_.player().complete())
        history_->addEntry("Все 10 кораблей расставлены. Можно сохранить флот и начать бой.");
    updateView();
}

void Window::undoPlacement() {
    if (manualMode_ && game_.undoPlacement())
        updateView();
}

void Window::saveFleet() {
    QString path = QFileDialog::getSaveFileName(
        this, "Сохранить расстановку", "fleet.txt", "Расстановка (*.txt)");
    if (path.isEmpty())
        return;

    QString error;
    if (!game_.save(path, error))
        QMessageBox::warning(this, "Ошибка сохранения", error);
    else
        history_->addEntry("Расстановка сохранена: " + QFileInfo(path).fileName());
}

void Window::loadFleet() {
    QString path = QFileDialog::getOpenFileName(
        this, "Загрузить расстановку", {}, "Расстановка (*.txt);;Все файлы (*)");
    if (path.isEmpty())
        return;

    QString error;
    if (!game_.load(path, error))
        QMessageBox::warning(this, "Ошибка загрузки", error);
    else {
        manualMode_ = false;
        history_->addEntry("Расстановка загружена: " + QFileInfo(path).fileName());
    }
    updateView();
}

void Window::chooseStrategy(int index) {
    game_.setStrategy(index);
}

void Window::changeHints(bool enabled) {
    playerBoard_->setHints(enabled);
    enemyBoard_->setHints(enabled);
}

void Window::recordShot(const QString& who, QPoint position, Field::Result result) {
    QString text;
    switch (result) {
        case Field::Result::Miss:
            text = "мимо";
            break;
        case Field::Result::Hit:
            text = "попадание, ещё один выстрел";
            break;
        case Field::Result::Sunk:
            text = "корабль потоплен";
            break;
        default:
            return;
    }

    history_->addEntry(QString("%1: %2%3 — %4")
                           .arg(who)
                           .arg(QChar('A' + position.x()))
                           .arg(position.y() + 1)
                           .arg(text));
    if (game_.phase() == Game::Phase::Won || game_.phase() == Game::Phase::Lost)
        history_->addEntry(game_.toString());
}

void Window::shoot(QPoint position) {
    recordShot("Вы", position, game_.shoot(position));
    updateView();
}

void Window::computerStep() {
    auto result = game_.computerTurn();
    recordShot("Компьютер", result.first, result.second);
    updateView();
}

void Window::openReports() {
    QString directory = QFileInfo(Session::reportPath()).absolutePath();
    if (!QDir().mkpath(directory) || !QDesktopServices::openUrl(QUrl::fromLocalFile(directory)))
        QMessageBox::information(this,
                                 "Журнал сеансов",
                                 "Журнал сохраняется при закрытии окна:\n" + Session::reportPath());
}

void Window::closeEvent(QCloseEvent* event) {
    timer_->stop();
    QString error;
    if (!session_.finish(error)) {
        QMessageBox::critical(this,
                              "Не удалось сохранить журнал",
                              error + "\n" + Session::reportPath() +
                                  "\nИсправьте причину и закройте окно ещё раз.");
        event->ignore();
        if (game_.phase() == Game::Phase::Computer)
            timer_->start();
        return;
    }

    event->accept();
}
