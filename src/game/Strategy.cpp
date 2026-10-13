#include "game/Strategy.h"

#include <QRandomGenerator>

#include <algorithm>
#include <vector>

bool HardStrategy::inside(int x, int y) {
    return x >= 0 && x < 10 && y >= 0 && y < 10;
}

std::array<int, 4> HardStrategy::remainingFleet(const std::array<int, 100>& shots) {
    std::array<int, 4> remaining{4, 3, 2, 1};
    std::array<bool, 100> visited{};

    for (int start = 0; start < 100; ++start) {
        if (shots[start] != 3 || visited[start])
            continue;

        std::vector<int> group{start};
        visited[start] = true;
        for (std::size_t index = 0; index < group.size(); ++index) {
            int x = group[index] % 10, y = group[index] / 10;
            for (QPoint step : {QPoint(1, 0), QPoint(-1, 0), QPoint(0, 1), QPoint(0, -1)}) {
                int nx = x + step.x(), ny = y + step.y();
                if (!inside(nx, ny))
                    continue;

                int neighbor = ny * 10 + nx;
                if (shots[neighbor] == 3 && !visited[neighbor]) {
                    visited[neighbor] = true;
                    group.push_back(neighbor);
                }
            }
        }

        if (group.size() <= 4 && remaining[group.size() - 1] > 0)
            --remaining[group.size() - 1];
    }
    return remaining;
}

std::array<bool, 100> HardStrategy::forbiddenCells(const std::array<int, 100>& shots) {
    std::array<bool, 100> forbidden{};

    for (int index = 0; index < 100; ++index) {
        if (shots[index] != 0 && shots[index] != 2)
            forbidden[index] = true;
        if (shots[index] != 2 && shots[index] != 3)
            continue;

        int x = index % 10, y = index / 10;
        for (int dy = -1; dy <= 1; ++dy)
            for (int dx = -1; dx <= 1; ++dx) {
                if (!inside(x + dx, y + dy))
                    continue;
                if (shots[index] == 3 || (dx != 0 && dy != 0))
                    forbidden[(y + dy) * 10 + x + dx] = true;
            }
    }
    return forbidden;
}

QPoint HardStrategy::next(const std::array<int, 100>& shots) const {
    auto remaining = remainingFleet(shots);
    auto forbidden = forbiddenCells(shots);
    std::array<int, 100> search{}, finish{};

    for (int length = 1; length <= 4; ++length) {
        if (remaining[length - 1] == 0)
            continue;

        for (int direction = 0; direction < (length == 1 ? 1 : 2); ++direction) {
            int dx = direction == 0 ? 1 : 0, dy = direction == 1 ? 1 : 0;
            for (int y = 0; y < 10; ++y)
                for (int x = 0; x < 10; ++x) {
                    if (!inside(x + dx * (length - 1), y + dy * (length - 1)))
                        continue;

                    std::array<int, 4> cells{};
                    int hits = 0;
                    bool valid = true;

                    for (int i = 0; i < length; ++i) {
                        cells[i] = (y + dy * i) * 10 + x + dx * i;
                        if (forbidden[cells[i]])
                            valid = false;
                        if (shots[cells[i]] == 2)
                            ++hits;
                    }

                    if (!valid || hits == length)
                        continue;

                    for (int i = 0; i < length && valid; ++i) {
                        int cx = cells[i] % 10, cy = cells[i] / 10;
                        for (int sy = -1; sy <= 1; ++sy)
                            for (int sx = -1; sx <= 1; ++sx) {
                                if (!inside(cx + sx, cy + sy))
                                    continue;

                                int neighbor = (cy + sy) * 10 + cx + sx;
                                if (shots[neighbor] == 2 &&
                                    std::find(cells.begin(), cells.begin() + length, neighbor) ==
                                        cells.begin() + length)
                                    valid = false;
                            }
                    }

                    if (!valid)
                        continue;

                    for (int i = 0; i < length; ++i)
                        if (shots[cells[i]] == 0) {
                            search[cells[i]] += remaining[length - 1];
                            finish[cells[i]] += remaining[length - 1] * hits * hits;
                        }
                }
        }
    }

    bool wounded = false;
    for (int score : finish)
        if (score > 0)
            wounded = true;

    const auto& scores = wounded ? finish : search;
    int best = 0;
    std::vector<QPoint> choices;

    for (int index = 0; index < 100; ++index) {
        if (shots[index] != 0 || forbidden[index] || scores[index] < best)
            continue;

        if (scores[index] > best) {
            best = scores[index];
            choices.clear();
        }
        choices.push_back({index % 10, index / 10});
    }

    if (choices.empty())
        return RandomStrategy().next(shots);
    return choices[QRandomGenerator::global()->bounded(static_cast<int>(choices.size()))];
}

QPoint RandomStrategy::next(const std::array<int, 100>& shots) const {
    std::vector<QPoint> available;
    for (int y = 0; y < 10; ++y)
        for (int x = 0; x < 10; ++x)
            if (shots[y * 10 + x] == 0)
                available.push_back({x, y});

    if (available.empty())
        return {-1, -1};
    return available[QRandomGenerator::global()->bounded(static_cast<int>(available.size()))];
}
