#include "game/Game.h"

Game::Game() : strategy_(std::make_unique<HardStrategy>()) {
    newGame();
}

bool Game::newGame() {
    Field player, computer;
    if (!player.randomize() || !computer.randomize())
        return false;

    player_ = player;
    computer_ = computer;
    phase_ = Phase::Setup;
    return true;
}

bool Game::shuffle() {
    return phase_ == Phase::Setup && player_.randomize();
}

bool Game::clearFleet() {
    if (phase_ != Phase::Setup)
        return false;

    player_.clear();
    return true;
}

int Game::nextShipLength() const {
    std::array<int, 4> counts{};
    for (const auto& item : player_.ships().entries())
        ++counts[item.second.length() - 1];

    for (int length = 4; length >= 1; --length)
        if (counts[length - 1] < 5 - length)
            return length;
    return 0;
}

bool Game::placeShip(QPoint position, bool horizontal) {
    int length = nextShipLength();
    return phase_ == Phase::Setup && player_.canPlace(position, length, horizontal) &&
           player_.add(Ship(position, length, horizontal));
}

bool Game::undoPlacement() {
    return phase_ == Phase::Setup && player_.removeLast();
}

bool Game::setStrategy(int index) {
    if (phase_ != Phase::Setup || index < 0 || index > 1)
        return false;

    if (index == 0)
        strategy_ = std::make_unique<HardStrategy>();
    else
        strategy_ = std::make_unique<RandomStrategy>();
    return true;
}

bool Game::start() {
    if (phase_ != Phase::Setup || !player_.complete() || !computer_.complete())
        return false;

    phase_ = Phase::Player;
    return true;
}

Field::Result Game::shoot(QPoint position) {
    if (phase_ != Phase::Player)
        return Field::Result::Invalid;

    auto result = computer_.shoot(position);
    if (!computer_)
        phase_ = Phase::Won;
    else if (result == Field::Result::Miss)
        phase_ = Phase::Computer;
    return result;
}

std::pair<QPoint, Field::Result> Game::computerTurn() {
    if (phase_ != Phase::Computer)
        return {{-1, -1}, Field::Result::Invalid};

    QPoint position = strategy_->next(player_.shots());
    auto result = player_.shoot(position);
    if (!player_)
        phase_ = Phase::Lost;
    else if (result == Field::Result::Miss)
        phase_ = Phase::Player;
    return {position, result};
}

bool Game::save(const QString& path, QString& error) const {
    if (phase_ != Phase::Setup) {
        error = "Сохранение доступно до начала боя.";
        return false;
    }
    return player_.save(path, error);
}

bool Game::load(const QString& path, QString& error) {
    if (phase_ != Phase::Setup) {
        error = "Загрузка доступна до начала боя.";
        return false;
    }
    return player_.load(path, error);
}

QString Game::repr() const {
    return QString("Game(%1;%2;%3;%4)")
        .arg(static_cast<int>(phase_))
        .arg(strategy_->repr(), player_.repr(), computer_.repr());
}

QString Game::toString() const {
    switch (phase_) {
        case Phase::Setup:
            return "Проверьте расстановку и начните бой";
        case Phase::Player:
            return "Ваш ход. Выберите клетку на поле противника";
        case Phase::Computer:
            return "Компьютер выбирает клетку…";
        case Phase::Won:
            return "Победа! Все корабли противника потоплены";
        case Phase::Lost:
            return "Вы проиграли. Попробуйте ещё раз";
    }
    return {};
}
