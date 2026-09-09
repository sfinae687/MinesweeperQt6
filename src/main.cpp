#include <QApplication>
#include <QMessageBox>

#include "minesweeperwidget.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    MineSweeperWidget window;
    window.setWindowTitle(QStringLiteral("扫雷"));

    QObject::connect(&window, &MineSweeperWidget::gameWon, [&window](int seconds) {
        QMessageBox::information(&window, QStringLiteral("扫雷"),
                                 QStringLiteral("恭喜你赢了！\n用时 %1 秒").arg(seconds));
    });
    QObject::connect(&window, &MineSweeperWidget::gameLost, [&window]() {
        QMessageBox::warning(&window, QStringLiteral("扫雷"),
                             QStringLiteral("踩到地雷了，再来一次！"));
    });

    window.show();
    return app.exec();
}
