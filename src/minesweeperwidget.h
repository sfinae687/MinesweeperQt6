#pragma once

#include <QPushButton>
#include <QVector>
#include <QWidget>

#include "board.h"

class QComboBox;
class QLabel;
class QTimer;
class QVBoxLayout;

class CellButton : public QPushButton {
    Q_OBJECT

public:
    using QPushButton::QPushButton;

signals:
    void rightClicked();

protected:
    void mousePressEvent(QMouseEvent* event) override;
};

class MineSweeperWidget : public QWidget {
    Q_OBJECT

public:
    enum class Difficulty { Beginner, Intermediate, Expert };

    explicit MineSweeperWidget(QWidget* parent = nullptr);

public slots:
    void newGame();
    void setDifficulty(Difficulty d);

signals:
    void gameWon(int seconds);
    void gameLost();

private slots:
    void onCellClicked(int r, int c);
    void onCellRightClicked(int r, int c);
    void onTick();

private:
    static void difficultyConfig(Difficulty d, int& rows, int& cols, int& mines);
    static int cellSize(Difficulty d);

    void buildBoard();
    void refreshButton(int r, int c);
    void refreshAll();
    void updateHeader();
    void startClock();
    void stopClock();

    QVBoxLayout* mainLayout_ = nullptr;
    QWidget* boardHost_ = nullptr;
    QComboBox* combo_ = nullptr;
    QPushButton* faceButton_ = nullptr;
    QLabel* mineLabel_ = nullptr;
    QLabel* timeLabel_ = nullptr;
    QTimer* timer_ = nullptr;

    QVector<CellButton*> buttons_;

    Difficulty difficulty_ = Difficulty::Beginner;
    Board board_;
    int seconds_ = 0;
    bool started_ = false;
    bool won_ = false;
    bool lost_ = false;
};
