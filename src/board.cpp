#include "board.h"

#include <algorithm>

Board::Board(int rows, int cols, int mineCount)
    : rng_(std::random_device{}())
{
    reset(rows, cols, mineCount);
}

void Board::reset(int rows, int cols, int mineCount)
{
    rows_ = rows;
    cols_ = cols;
    mineCount_ = std::min(mineCount, rows * cols - 9);
    flagged_ = 0;
    revealedCount_ = 0;
    minesPlaced_ = false;
    started_ = false;
    exploded_ = false;
    boomR_ = -1;
    boomC_ = -1;
    cells_.assign(static_cast<size_t>(rows) * cols, Cell{});
}

void Board::placeMines(int safeR, int safeC)
{
    std::vector<int> candidates;
    candidates.reserve(static_cast<size_t>(rows_) * cols_ - 9);

    for (int r = 0; r < rows_; ++r) {
        for (int c = 0; c < cols_; ++c) {
            if (std::abs(r - safeR) <= 1 && std::abs(c - safeC) <= 1)
                continue;
            candidates.push_back(r * cols_ + c);
        }
    }

    std::shuffle(candidates.begin(), candidates.end(), rng_);
    for (int i = 0; i < mineCount_ && i < static_cast<int>(candidates.size()); ++i) {
        const int idx = candidates[i];
        cells_[static_cast<size_t>(idx)].mine = true;
    }
    minesPlaced_ = true;
}

int Board::neighborMines(int r, int c) const
{
    int n = 0;
    for (int dr = -1; dr <= 1; ++dr) {
        for (int dc = -1; dc <= 1; ++dc) {
            if (dr == 0 && dc == 0)
                continue;
            const int nr = r + dr;
            const int nc = c + dc;
            if (inBounds(nr, nc) && at(nr, nc).mine)
                ++n;
        }
    }
    return n;
}

int Board::neighborFlags(int r, int c) const
{
    int n = 0;
    for (int dr = -1; dr <= 1; ++dr) {
        for (int dc = -1; dc <= 1; ++dc) {
            if (dr == 0 && dc == 0)
                continue;
            const int nr = r + dr;
            const int nc = c + dc;
            if (inBounds(nr, nc) && at(nr, nc).marker == Marker::Flag)
                ++n;
        }
    }
    return n;
}

void Board::revealCellAt(int r, int c)
{
    Cell& cell = at(r, c);
    if (!cell.revealed) {
        cell.revealed = true;
        cell.marker = Marker::None;
        ++revealedCount_;
    }
}

void Board::flood(int r, int c)
{
    revealCellAt(r, c);

    const int mines = neighborMines(r, c);
    if (mines > 0)
        return;

    for (int dr = -1; dr <= 1; ++dr) {
        for (int dc = -1; dc <= 1; ++dc) {
            if (dr == 0 && dc == 0)
                continue;
            const int nr = r + dr;
            const int nc = c + dc;
            if (!inBounds(nr, nc))
                continue;
            Cell& n = at(nr, nc);
            if (n.revealed || n.marker == Marker::Flag)
                continue;
            if (n.mine)
                continue;
            flood(nr, nc);
        }
    }
}

void Board::revealOne(int r, int c)
{
    if (!inBounds(r, c))
        return;

    Cell& cell = at(r, c);
    if (cell.revealed || cell.marker == Marker::Flag)
        return;
    if (cell.mine)
        return;

    revealCellAt(r, c);
}

bool Board::reveal(int r, int c)
{
    if (!inBounds(r, c))
        return false;

    Cell& cell = at(r, c);
    if (cell.revealed || cell.marker == Marker::Flag)
        return false;

    if (!minesPlaced_)
        placeMines(r, c);

    started_ = true;

    if (cell.mine) {
        exploded_ = true;
        boomR_ = r;
        boomC_ = c;
        revealCellAt(r, c);
        revealAll();
        return true;
    }

    flood(r, c);
    return false;
}

bool Board::chord(int r, int c)
{
    if (!inBounds(r, c))
        return false;

    Cell& cell = at(r, c);
    if (!cell.revealed || cell.mine)
        return false;

    const int mines = neighborMines(r, c);
    if (mines == 0 || neighborFlags(r, c) != mines)
        return false;

    bool hitMine = false;
    for (int dr = -1; dr <= 1; ++dr) {
        for (int dc = -1; dc <= 1; ++dc) {
            if (dr == 0 && dc == 0)
                continue;
            const int nr = r + dr;
            const int nc = c + dc;
            if (!inBounds(nr, nc))
                continue;
            Cell& n = at(nr, nc);
            if (n.revealed || n.marker == Marker::Flag)
                continue;
            if (n.mine) {
                exploded_ = true;
                boomR_ = nr;
                boomC_ = nc;
                n.revealed = true;
                n.marker = Marker::None;
                ++revealedCount_;
                hitMine = true;
            } else {
                revealOne(nr, nc);
            }
        }
    }
    if (hitMine)
        revealAll();
    return hitMine;
}

void Board::toggleFlag(int r, int c)
{
    if (!inBounds(r, c))
        return;

    Cell& cell = at(r, c);
    if (cell.revealed)
        return;

    if (cell.marker == Marker::Flag) {
        cell.marker = Marker::None;
        --flagged_;
    } else if (flagged_ < mineCount_) {
        cell.marker = Marker::Flag;
        ++flagged_;
    }
}

void Board::revealAll()
{
    for (int r = 0; r < rows_; ++r) {
        for (int c = 0; c < cols_; ++c) {
            Cell& cell = at(r, c);
            cell.wrongFlag = cell.marker == Marker::Flag && !cell.mine;
            if (cell.mine && cell.marker != Marker::Flag) {
                if (!cell.revealed) {
                    cell.revealed = true;
                    ++revealedCount_;
                }
            } else if (!cell.mine && !cell.revealed) {
                cell.revealed = true;
                ++revealedCount_;
            }
        }
    }
}

bool Board::isWin() const
{
    return minesPlaced_ && revealedCount_ == rows_ * cols_ - mineCount_;
}
