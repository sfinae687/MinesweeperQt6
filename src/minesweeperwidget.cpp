#include "minesweeperwidget.h"

#include <QComboBox>
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

namespace {

const char* numberColor(int n)
{
    static const char* colors[] = {
        "#0000ff", "#008000", "#ff0000", "#000080",
        "#800000", "#008080", "#000000", "#808080"
    };
    if (n < 1 || n > 8)
        return "#000000";
    return colors[n - 1];
}

} // namespace

void CellButton::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton) {
        emit rightClicked();
        event->accept();
        return;
    }
    QPushButton::mousePressEvent(event);
}

MineSweeperWidget::MineSweeperWidget(QWidget* parent)
    : QWidget(parent)
{
    mainLayout_ = new QVBoxLayout(this);
    mainLayout_->setContentsMargins(0, 0, 0, 0);
    mainLayout_->setSpacing(0);

    auto* header = new QWidget(this);
    auto* headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(8, 4, 8, 4);
    headerLayout->setSpacing(12);

    combo_ = new QComboBox(header);
    combo_->addItem(QStringLiteral("初级 (9×9, 10雷)"));
    combo_->addItem(QStringLiteral("中级 (16×16, 40雷)"));
    combo_->addItem(QStringLiteral("高级 (30×16, 99雷)"));
    connect(combo_, &QComboBox::currentIndexChanged, this, [this](int index) {
        setDifficulty(static_cast<Difficulty>(index));
    });

    mineLabel_ = new QLabel(header);
    mineLabel_->setStyleSheet(QStringLiteral("font-weight:bold; font-size:18px;"));

    faceButton_ = new QPushButton(header);
    faceButton_->setFixedSize(44, 44);
    faceButton_->setStyleSheet(QStringLiteral("font-size:24px;"));
    connect(faceButton_, &QPushButton::clicked, this, &MineSweeperWidget::newGame);

    timeLabel_ = new QLabel(header);
    timeLabel_->setStyleSheet(QStringLiteral("font-weight:bold; font-size:18px;"));

    headerLayout->addWidget(combo_);
    headerLayout->addStretch();
    headerLayout->addWidget(mineLabel_);
    headerLayout->addWidget(faceButton_);
    headerLayout->addWidget(timeLabel_);
    headerLayout->addStretch();

    auto* tip = new QLabel(QStringLiteral("左键翻开 | 右键插旗 | 点击已翻开的数字可展开周围"), header);
    tip->setStyleSheet(QStringLiteral("color:gray;"));
    headerLayout->addWidget(tip);

    mainLayout_->addWidget(header);

    timer_ = new QTimer(this);
    timer_->setInterval(1000);
    connect(timer_, &QTimer::timeout, this, &MineSweeperWidget::onTick);

    newGame();
}

void MineSweeperWidget::difficultyConfig(Difficulty d, int& rows, int& cols, int& mines)
{
    switch (d) {
    case Difficulty::Beginner:
        rows = 9;
        cols = 9;
        mines = 10;
        break;
    case Difficulty::Intermediate:
        rows = 16;
        cols = 16;
        mines = 40;
        break;
    case Difficulty::Expert:
        rows = 16;
        cols = 30;
        mines = 99;
        break;
    }
}

int MineSweeperWidget::cellSize(Difficulty d)
{
    switch (d) {
    case Difficulty::Beginner:
        return 40;
    case Difficulty::Intermediate:
        return 36;
    case Difficulty::Expert:
        return 30;
    }
    return 36;
}

void MineSweeperWidget::setDifficulty(Difficulty d)
{
    difficulty_ = d;
    newGame();
}

void MineSweeperWidget::newGame()
{
    stopClock();
    started_ = false;
    won_ = false;
    lost_ = false;
    seconds_ = 0;
    faceButton_->setText(QStringLiteral("🙂"));

    int rows, cols, mines;
    difficultyConfig(difficulty_, rows, cols, mines);
    board_.reset(rows, cols, mines);

    buildBoard();
    refreshAll();
    updateHeader();
    adjustSize();
}

void MineSweeperWidget::buildBoard()
{
    if (boardHost_) {
        mainLayout_->removeWidget(boardHost_);
        delete boardHost_;
        boardHost_ = nullptr;
    }
    buttons_.clear();

    const int rows = board_.rows();
    const int cols = board_.cols();
    const int s = cellSize(difficulty_);

    boardHost_ = new QWidget(this);
    boardHost_->setFixedSize(cols * s, rows * s);

    buttons_.reserve(rows * cols);
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            auto* btn = new CellButton(boardHost_);
            btn->setGeometry(c * s, r * s, s, s);
            btn->setFocusPolicy(Qt::NoFocus);
            QFont f = btn->font();
            f.setPixelSize(qMax(14, s - 12));
            f.setBold(true);
            btn->setFont(f);

            const int rr = r;
            const int cc = c;
            connect(btn, &QPushButton::clicked, this, [this, rr, cc]() {
                onCellClicked(rr, cc);
            });
            connect(btn, &CellButton::rightClicked, this, [this, rr, cc]() {
                onCellRightClicked(rr, cc);
            });

            buttons_.append(btn);
        }
    }

    mainLayout_->addWidget(boardHost_, 0, Qt::AlignLeft | Qt::AlignTop);
}

void MineSweeperWidget::refreshAll()
{
    for (int r = 0; r < board_.rows(); ++r)
        for (int c = 0; c < board_.cols(); ++c)
            refreshButton(r, c);
}

void MineSweeperWidget::refreshButton(int r, int c)
{
    CellButton* btn = buttons_[r * board_.cols() + c];
    const Cell& cell = board_.at(r, c);

    if (cell.wrongFlag) {
        btn->setText(QStringLiteral("💣"));
        btn->setStyleSheet(QStringLiteral(
            "background:#ff8a8a; border:1px solid #cc0000; padding:0; margin:0; border-radius:0;"));
        return;
    }

    if (!cell.revealed) {
        btn->setText(cell.marker == Marker::Flag ? QStringLiteral("🚩") : QString());
        btn->setStyleSheet(QStringLiteral(
            "QPushButton {"
            "  background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #f7f7f7,stop:1 #bdbdbd);"
            "  border: 1px solid #8a8a8a;"
            "  border-radius: 0;"
            "  padding: 0; margin: 0;"
            "}"));
        return;
    }

    QString style = QStringLiteral(
        "background:#d0d0d0; border:1px solid #9a9a9a; padding:0; margin:0; border-radius:0;");
    QString text;
    if (cell.mine) {
        text = QStringLiteral("💣");
    } else {
        const int n = board_.neighborMines(r, c);
        if (n > 0) {
            text = QString::number(n);
            style += QStringLiteral(" color:%1;").arg(numberColor(n));
        }
    }
    if (lost_ && r == board_.boomRow() && c == board_.boomCol())
        style.replace("background:#d0d0d0;", "background:#ff6a6a;");
    btn->setText(text);
    btn->setStyleSheet(style);
}

void MineSweeperWidget::updateHeader()
{
    mineLabel_->setText(QStringLiteral("🚩 %1").arg(board_.mineCount() - board_.flagsPlaced()));
    timeLabel_->setText(QStringLiteral("⏱ %1").arg(seconds_));
}

void MineSweeperWidget::onTick()
{
    if (seconds_ < 999)
        ++seconds_;
    updateHeader();
}

void MineSweeperWidget::startClock()
{
    if (!timer_->isActive()) {
        seconds_ = 0;
        timer_->start();
    }
}

void MineSweeperWidget::stopClock()
{
    timer_->stop();
}

void MineSweeperWidget::onCellClicked(int r, int c)
{
    if (won_ || lost_)
        return;

    const Cell& cell = board_.at(r, c);
    if (cell.marker == Marker::Flag)
        return;

    if (!started_) {
        started_ = true;
        startClock();
    }

    bool hitMine = cell.revealed ? board_.chord(r, c) : board_.reveal(r, c);

    if (hitMine) {
        lost_ = true;
        stopClock();
        faceButton_->setText(QStringLiteral("😵"));
        emit gameLost();
    } else if (board_.isWin()) {
        won_ = true;
        stopClock();
        faceButton_->setText(QStringLiteral("😎"));
        emit gameWon(seconds_);
    }

    refreshAll();
    updateHeader();
}

void MineSweeperWidget::onCellRightClicked(int r, int c)
{
    if (won_ || lost_)
        return;

    board_.toggleFlag(r, c);
    refreshButton(r, c);
    updateHeader();
}
