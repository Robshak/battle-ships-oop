#include "ui/Widgets.h"

#include <QApplication>
#include <QClipboard>
#include <QFocusEvent>
#include <QKeyEvent>
#include <QLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QToolTip>
#include <QWheelEvent>

#include <algorithm>

TitleLabel::TitleLabel(const QString& text, QWidget* parent) : QLabel(text, parent) {
    setWordWrap(true);
}

void TitleLabel::mouseDoubleClickEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        QApplication::clipboard()->setText(text());
        QToolTip::showText(event->globalPosition().toPoint(), "Текст скопирован", this);
    }

    QLabel::mouseDoubleClickEvent(event);
}

ActionButton::ActionButton(const QString& text, QWidget* parent) : QPushButton(text, parent) {
    setMinimumHeight(32);
    setCursor(Qt::PointingHandCursor);
}

void ActionButton::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        click();
        event->accept();
    } else
        QPushButton::keyPressEvent(event);
}

HistoryScrollBar::HistoryScrollBar(QWidget* parent) : QScrollBar(Qt::Vertical, parent) {
    setFocusPolicy(Qt::StrongFocus);
}

void HistoryScrollBar::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        setValue(maximum());
        event->accept();
    } else
        QScrollBar::keyPressEvent(event);
}

StrategyCombo::StrategyCombo(QWidget* parent) : QComboBox(parent) {
    addItems({"Сложная — поиск и добивание", "Простая — случайные выстрелы"});
    setCurrentIndex(0);
    setFocusPolicy(Qt::StrongFocus);
}

void StrategyCombo::wheelEvent(QWheelEvent* event) {
    if (hasFocus())
        QComboBox::wheelEvent(event);
    else
        event->ignore();
}

HintCheck::HintCheck(QWidget* parent) : QCheckBox("Подсказки координат", parent) {
    setChecked(true);
    setToolTip("Показывать координаты при наведении. Клавиша H переключает подсказки.");
}

void HintCheck::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_H) {
        toggle();
        event->accept();
    } else
        QCheckBox::keyPressEvent(event);
}

FleetProgress::FleetProgress(QWidget* parent) : QProgressBar(parent) {
    setRange(0, 20);
    setValue(20);
    setFormat("%v из %m палуб");
    setAlignment(Qt::AlignCenter);
    setMinimumHeight(22);
}

void FleetProgress::mousePressEvent(QMouseEvent* event) {
    QToolTip::showText(event->globalPosition().toPoint(), toString(), this);
    QProgressBar::mousePressEvent(event);
}

EventList::EventList(QWidget* parent) : QListWidget(parent) {
    setVerticalScrollBar(new HistoryScrollBar(this));
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setMinimumHeight(88);
    setMaximumHeight(125);
}

void EventList::addEntry(const QString& message) {
    addItem(message);
    if (count() > 500)
        delete takeItem(0);
    scrollToBottom();
}

QString EventList::repr() const {
    QString value = "EventList[";
    for (int i = 0; i < count(); ++i)
        value += item(i)->text() + "\n";
    return value + "]";
}

void EventList::keyPressEvent(QKeyEvent* event) {
    if (event->matches(QKeySequence::Copy)) {
        QStringList lines;
        for (const QListWidgetItem* item : selectedItems())
            lines << item->text();
        QApplication::clipboard()->setText(lines.join('\n'));
        event->accept();
    } else
        QListWidget::keyPressEvent(event);
}

BoardGroup::BoardGroup(const QString& title, QWidget* parent) : QGroupBox(title, parent) {
}

void BoardGroup::mouseDoubleClickEvent(QMouseEvent* event) {
    if (layout())
        for (int i = 0; i < layout()->count(); ++i)
            if (auto* board = dynamic_cast<FieldBoard*>(layout()->itemAt(i)->widget()))
                board->setFocus();

    QGroupBox::mouseDoubleClickEvent(event);
}

FieldBoard::FieldBoard(QWidget* parent) : QWidget(parent) {
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumSize(minimumSizeHint());
}

QRect FieldBoard::gridRect() const {
    int cell = std::max(1, (std::min(width(), height()) - 34) / 10);
    int side = cell * 10;
    return {(width() - side) / 2 + 10, (height() - side) / 2 + 10, side, side};
}

QPoint FieldBoard::cellAt(QPoint position) const {
    QRect grid = gridRect();
    if (!grid.contains(position))
        return {-1, -1};

    int cell = grid.width() / 10;
    return {(position.x() - grid.x()) / cell, (position.y() - grid.y()) / cell};
}

void FieldBoard::setCanShoot(bool value) {
    canShoot_ = value;
    setCursor(canShoot_ || placementLength_ ? Qt::CrossCursor : Qt::ArrowCursor);
    update();
}

void FieldBoard::setPlacement(int length, bool horizontal) {
    placementLength_ = length >= 1 && length <= 4 ? length : 0;
    horizontal_ = horizontal;
    setCursor(canShoot_ || placementLength_ ? Qt::CrossCursor : Qt::ArrowCursor);
    update();
}

bool FieldBoard::setSelected(QPoint position) {
    if (!Field::inside(position))
        return false;

    selected_ = position;
    update();
    return true;
}

void FieldBoard::activate() {
    if (placementLength_ && onPlace_)
        onPlace_(selected_);
    else if (canShoot_ && field_ && field_->shotAt(selected_) == 0 && onShot_)
        onShot_(selected_);
}

void FieldBoard::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRect grid = gridRect();
    int size = grid.width() / 10;

    painter.setPen(QColor("#34445b"));
    for (int i = 0; i < 10; ++i) {
        painter.drawText(QRect(grid.x() + i * size, grid.y() - 24, size, 22),
                         Qt::AlignCenter,
                         QString(QChar('A' + i)));
        painter.drawText(QRect(grid.x() - 26, grid.y() + i * size, 24, size),
                         Qt::AlignCenter,
                         QString::number(i + 1));
    }

    for (int y = 0; y < 10; ++y)
        for (int x = 0; x < 10; ++x) {
            QPoint position(x, y);
            QRect cell(grid.x() + x * size, grid.y() + y * size, size, size);
            int mark = field_ ? field_->shotAt(position) : 0;
            bool ship = field_ && field_->shipAt(position);
            QColor fill = ship && revealed_ ? QColor("#668bad") : QColor("#f4f8fc");
            if (mark == 3)
                fill = QColor("#e9a5a5");

            painter.fillRect(cell, fill);
            painter.setPen(QColor("#b8c9d8"));
            painter.setBrush(Qt::NoBrush);
            painter.drawRect(cell);

            QPoint center = cell.center();
            if (mark == 1) {
                painter.setBrush(QColor("#55758f"));
                painter.setPen(Qt::NoPen);
                painter.drawEllipse(center, 3, 3);
            } else if (mark >= 2) {
                painter.setPen(QPen(QColor("#aa2d3a"), 2.5));
                int radius = size / 4;
                painter.drawLine(center + QPoint(-radius, -radius),
                                 center + QPoint(radius, radius));
                painter.drawLine(center + QPoint(-radius, radius),
                                 center + QPoint(radius, -radius));
            }

            if (canShoot_ && hasFocus() && selected_ == position) {
                painter.setPen(QPen(QColor("#e4a538"), 3));
                painter.setBrush(Qt::NoBrush);
                painter.drawRect(cell.adjusted(2, 2, -2, -2));
            }
        }

    if (placementLength_ && field_) {
        bool valid = field_->canPlace(selected_, placementLength_, horizontal_);
        painter.setPen(QPen(QColor(valid ? "#22814f" : "#bc3644"), 2));
        painter.setBrush(QColor(valid ? "#882da86b" : "#88db5461"));

        for (int i = 0; i < placementLength_; ++i) {
            QPoint cell = selected_ + (horizontal_ ? QPoint(i, 0) : QPoint(0, i));
            if (Field::inside(cell))
                painter.drawRect(
                    QRect(grid.x() + cell.x() * size, grid.y() + cell.y() * size, size, size)
                        .adjusted(2, 2, -2, -2));
        }
    }
}

void FieldBoard::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::RightButton && placementLength_ && onRotate_) {
        onRotate_();
        event->accept();
        return;
    }

    if (event->button() == Qt::LeftButton && setSelected(cellAt(event->position().toPoint()))) {
        setFocus();
        activate();
    }

    QWidget::mousePressEvent(event);
}

void FieldBoard::mouseMoveEvent(QMouseEvent* event) {
    QPoint position = cellAt(event->position().toPoint());
    if (placementLength_)
        setSelected(position);

    if (hints_ && Field::inside(position))
        setToolTip(QString("%1%2").arg(QChar('A' + position.x())).arg(position.y() + 1));
    else
        setToolTip({});

    QWidget::mouseMoveEvent(event);
}

void FieldBoard::keyPressEvent(QKeyEvent* event) {
    QPoint step;
    switch (event->key()) {
        case Qt::Key_Left:
            step = {-1, 0};
            break;
        case Qt::Key_Right:
            step = {1, 0};
            break;
        case Qt::Key_Up:
            step = {0, -1};
            break;
        case Qt::Key_Down:
            step = {0, 1};
            break;
        case Qt::Key_R:
            if (placementLength_ && onRotate_)
                onRotate_();
            event->accept();
            return;
        case Qt::Key_Return:
        case Qt::Key_Enter:
        case Qt::Key_Space:
            activate();
            event->accept();
            return;
        default:
            QWidget::keyPressEvent(event);
            return;
    }

    setSelected(selected_ + step);
    event->accept();
}

void FieldBoard::focusInEvent(QFocusEvent* event) {
    QWidget::focusInEvent(event);
    update();
}

void FieldBoard::focusOutEvent(QFocusEvent* event) {
    QWidget::focusOutEvent(event);
    update();
}

QString FieldBoard::repr() const {
    return QString("FieldBoard(%1,%2,%3,%4,%5;%6)")
        .arg(revealed_)
        .arg(canShoot_)
        .arg(hints_)
        .arg(placementLength_)
        .arg(horizontal_)
        .arg(field_ ? field_->repr() : "empty");
}
