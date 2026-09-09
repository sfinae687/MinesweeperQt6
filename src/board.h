#pragma once

#include <random>
#include <vector>

enum class Marker { None, Flag };

struct Cell {
    bool mine = false;
    bool revealed = false;
    bool wrongFlag = false;
    Marker marker = Marker::None;
};

class Board {
public:
    Board(int rows = 9, int cols = 9, int mineCount = 10);

    void reset(int rows, int cols, int mineCount);

    int rows() const { return rows_; }
    int cols() const { return cols_; }
    int mineCount() const { return mineCount_; }
    bool minesPlaced() const { return minesPlaced_; }
    bool started() const { return started_; }
    bool exploded() const { return exploded_; }
    int boomRow() const { return boomR_; }
    int boomCol() const { return boomC_; }

    const Cell& at(int r, int c) const { return cells_[static_cast<size_t>(r) * cols_ + c]; }
    Cell& at(int r, int c) { return cells_[static_cast<size_t>(r) * cols_ + c]; }

    int neighborMines(int r, int c) const;
    int neighborFlags(int r, int c) const;
    int flagsPlaced() const { return flagged_; }
    int revealedCount() const { return revealedCount_; }

    bool isWin() const;

    bool reveal(int r, int c);
    bool chord(int r, int c);
    void toggleFlag(int r, int c);
    void revealAll();

private:
    bool inBounds(int r, int c) const {
        return r >= 0 && r < rows_ && c >= 0 && c < cols_;
    }

    void placeMines(int safeR, int safeC);
    void flood(int r, int c);
    void revealOne(int r, int c);
    void revealCellAt(int r, int c);

    int rows_ = 0;
    int cols_ = 0;
    int mineCount_ = 0;
    int flagged_ = 0;
    int revealedCount_ = 0;
    bool minesPlaced_ = false;
    bool started_ = false;
    bool exploded_ = false;
    int boomR_ = -1;
    int boomC_ = -1;
    std::vector<Cell> cells_;
    std::mt19937 rng_;
};
