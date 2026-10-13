#include "model/Field.h"

#include <QFile>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QSaveFile>
#include <QTextStream>

bool Field::inside(QPoint position) {
    return position.x() >= 0 && position.x() < 10 && position.y() >= 0 && position.y() < 10;
}

const Ship* Field::shipAt(QPoint position) const {
    for (const auto& item : ships_.entries())
        if (item.second.contains(position))
            return &item.second;
    return nullptr;
}

int Field::shotAt(QPoint position) const {
    return inside(position) ? shots_[position.y() * 10 + position.x()] : -1;
}

bool Field::add(const Ship& ship) {
    return ship.hits() == 0 && canPlace(ship.position(), ship.length(), ship.horizontal()) &&
           ships_.insert(ship);
}

bool Field::canPlace(QPoint position, int length, bool horizontal) const {
    if (!inside(position) || length < 1 || length > 4)
        return false;

    for (int shot : shots_)
        if (shot != 0)
            return false;

    for (int i = 0; i < length; ++i) {
        QPoint cell = position + (horizontal ? QPoint(i, 0) : QPoint(0, i));
        if (!inside(cell))
            return false;
        for (int dy = -1; dy <= 1; ++dy)
            for (int dx = -1; dx <= 1; ++dx)
                if (shipAt(cell + QPoint(dx, dy)))
                    return false;
    }
    return true;
}

bool Field::removeLast() {
    for (int shot : shots_)
        if (shot != 0)
            return false;
    return ships_.removeLast();
}

void Field::clear() {
    ships_.clear();
    shots_.fill(0);
}

bool Field::randomize() {
    auto* random = QRandomGenerator::global();

    for (int retry = 0; retry < 200; ++retry) {
        Field next;
        for (int length : {4, 3, 3, 2, 2, 2, 1, 1, 1, 1}) {
            bool placed = false;
            for (int attempt = 0; attempt < 300 && !placed; ++attempt) {
                bool horizontal = random->bounded(2) == 0;
                QPoint position(random->bounded(horizontal ? 11 - length : 10),
                                random->bounded(horizontal ? 10 : 11 - length));
                placed = next.add(Ship(position, length, horizontal));
            }
            if (!placed)
                break;
        }

        if (next.complete()) {
            *this = next;
            return true;
        }
    }
    return false;
}

bool Field::complete() const {
    std::array<int, 4> counts{};
    for (const auto& item : ships_.entries())
        ++counts[item.second.length() - 1];
    return counts == std::array<int, 4>{4, 3, 2, 1};
}

int Field::remainingShips() const {
    int count = 0;
    for (const auto& item : ships_.entries())
        if (item.second)
            ++count;
    return count;
}

int Field::remainingDecks() const {
    int count = 0;
    for (const auto& item : ships_.entries())
        count += item.second.remaining();
    return count;
}

Field::Result Field::shoot(QPoint position) {
    if (!inside(position))
        return Result::Invalid;

    int& mark = shots_[position.y() * 10 + position.x()];
    if (mark != 0)
        return Result::Repeat;

    const Ship* ship = shipAt(position);
    if (!ship) {
        mark = 1;
        return Result::Miss;
    }

    ships_.hit(ship->position(), position);
    mark = 2;
    if (*ship)
        return Result::Hit;

    for (int i = 0; i < ship->length(); ++i) {
        QPoint cell = ship->cell(i);
        shots_[cell.y() * 10 + cell.x()] = 3;
    }
    return Result::Sunk;
}

bool Field::save(const QString& path, QString& error) const {
    if (!complete() || remainingDecks() != 20) {
        error = "Сохранить можно только начальную расстановку.";
        return false;
    }

    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        error = file.errorString();
        return false;
    }

    QTextStream stream(&file);
    stream << "10 10\n";
    for (const auto& item : ships_.entries()) {
        const Ship& ship = item.second;
        stream << ship.length() << ' ' << (ship.horizontal() ? 'h' : 'v') << ' '
               << ship.position().x() << ' ' << ship.position().y() << '\n';
    }

    stream.flush();
    if (stream.status() != QTextStream::Ok || !file.commit()) {
        error = file.errorString();
        return false;
    }
    return true;
}

bool Field::load(const QString& path, QString& error) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        error = file.errorString();
        return false;
    }
    if (file.size() > 4096) {
        error = "Файл расстановки слишком большой.";
        return false;
    }

    QStringList lines = QString::fromUtf8(file.readAll()).split('\n', Qt::SkipEmptyParts);
    if (file.error() != QFile::NoError) {
        error = file.errorString();
        return false;
    }

    Field next;
    QRegularExpression spaces("\\s+");
    if (lines.isEmpty() || lines.takeFirst().trimmed().split(spaces) != QStringList{"10", "10"}) {
        error = "В первой строке должны быть размеры поля: 10 10.";
        return false;
    }

    for (const QString& line : lines) {
        if (line.trimmed().isEmpty())
            continue;

        QStringList values = line.trimmed().split(spaces);
        bool lengthOk = false, xOk = false, yOk = false;
        int length = values.value(0).toInt(&lengthOk);
        int x = values.value(2).toInt(&xOk), y = values.value(3).toInt(&yOk);
        bool horizontal = values.value(1) == "h";
        bool valid = values.size() == 4 && lengthOk && xOk && yOk && length >= 1 && length <= 4 &&
                     (values.value(1) == "h" || values.value(1) == "v") && inside({x, y});

        if (valid) {
            QPoint end(x + (horizontal ? length - 1 : 0), y + (horizontal ? 0 : length - 1));
            valid = inside(end) && next.add(Ship({x, y}, length, horizontal));
        }

        if (!valid) {
            error = "Неверная строка, пересечение или соприкосновение кораблей: " + line;
            return false;
        }
    }

    if (!next.complete()) {
        error = "Нужны 4 однопалубных, 3 двухпалубных, 2 трёхпалубных и 1 четырёхпалубный корабль.";
        return false;
    }

    if (*this == next)
        return true;

    *this = next;
    return true;
}

QString Field::repr() const {
    QString value = "Field(" + ships_.repr() + ";";
    for (int shot : shots_)
        value += QString::number(shot);
    return value + ")";
}

QString Field::toString() const {
    return QString("Осталось кораблей: %1, палуб: %2").arg(remainingShips()).arg(remainingDecks());
}
