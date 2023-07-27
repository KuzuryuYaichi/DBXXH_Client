#include <QApplication>
#include "Model.h"
#include "DataManager.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    DataManager manager;
    manager.load();
    Model m;
    m.showMaximized();
    return a.exec();
}
