#include "model/Ship.h"

#include <stdexcept>

Ship::Ship(QPoint position, int length, bool horizontal) {
    if (!setPlacement(position, length, horizontal))
        throw std::invalid_argument("Invalid ship");
}

bool Ship::setPlacement(QPoint position, int length, bool horizontal) {
    if (length < 1 || length > 4 || position.x() < 0 || position.x() >= 10 || position.y() < 0 ||
        position.y() >= 10 || hits_ != 0)
        return false;

    QPoint end = position + (horizontal ? QPoint(length - 1, 0) : QPoint(0, length - 1));
    if (end.x() >= 10 || end.y() >= 10)
        return false;

    position_ = position;
    length_ = length;
    horizontal_ = horizontal;
    return true;
}

QPoint Ship::cell(int index) const {
    if (index < 0 || index >= length_)
        return {-1, -1};
    return position_ + (horizontal_ ? QPoint(index, 0) : QPoint(0, index));
}

bool Ship::contains(QPoint position) const {
    for (int i = 0; i < length_; ++i)
        if (cell(i) == position)
            return true;
    return false;
}

bool Ship::hit(QPoint position) {
    if (!contains(position))
        return false;

    int index = horizontal_ ? position.x() - position_.x() : position.y() - position_.y();
    unsigned mask = 1u << index;
    if (hits_ & mask)
        return false;

    hits_ |= mask;
    return true;
}

int Ship::remaining() const {
    int count = 0;
    for (int i = 0; i < length_; ++i)
        if (!(hits_ & (1u << i)))
            ++count;
    return count;
}

QString Ship::repr() const {
    return QString("Ship(%1,%2,%3,%4,%5)")
        .arg(position_.x())
        .arg(position_.y())
        .arg(length_)
        .arg(horizontal_)
        .arg(hits_);
}

QString Ship::toString() const {
    return QString("Корабль: %1 палуб, целых %2").arg(length_).arg(remaining());
}
