#include <QApplication>
#include "MainWindow.hpp"

int main(int argc, char *argv[]) 
{
    QApplication app(argc, argv);
    app.setStyleSheet("QMainWindow { background-color: #1e1e2e; } "
                      "QLabel { color: #cdd6f4; } "
                      "QPushButton { background-color: #89b4fa; color: #11111b; border-radius: 5px; padding: 5px; }");
    MainWindow window;
    window.show();
    return app.exec();
}