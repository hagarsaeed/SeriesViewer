#include "SeriesViewer.h"
#include <QtWidgets/QApplication>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    // Initialize Google Test


    //set the app style sheet
    QFile styleSheetFile("C:\\Users\\hp\\Desktop\\SeriesViewer\\Gravira.qss");
    styleSheetFile.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(styleSheetFile.readAll());
    a.setStyleSheet(styleSheet);

    SeriesViewer w;
    w.show();

    return a.exec();
}
