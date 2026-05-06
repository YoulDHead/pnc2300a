#include <QApplication>
#include <QFile>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("CAMM-Grinder");
    app.setOrganizationName("CNC Tools");
    app.setApplicationVersion(APP_VERSION);

    QFile styleFile(":/styles.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString style = QLatin1String(styleFile.readAll());
        app.setStyleSheet(style);
    }

    MainWindow window;
    window.show();

    return app.exec();
}
