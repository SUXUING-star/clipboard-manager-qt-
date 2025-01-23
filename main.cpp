// main.cpp
#include "mainwindow.h"
#include <QApplication>
#include <QIcon>

int main(int argc, char *argv[]) {
    try {
        QApplication app(argc, argv);
        app.setWindowIcon(QIcon(":/icons/icon.ico"));
        MainWindow w;
        w.show();
        qDebug() << "Window shown";
        return app.exec();
    } catch (const std::exception& e) {
        qDebug() << "Exception:" << e.what();
        return 1;
    }
}
