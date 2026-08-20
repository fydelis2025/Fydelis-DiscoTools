#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("FydelisDisk Suite");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("FydelisTechOS");

    MainWindow w;
    w.show();
    return app.exec();
}
