#include <QApplication>
#include "Model.h"
#include "DataManager.h"

bool ProcessExisted()
{
    QLockFile lockFile(QApplication::applicationDirPath() + "/LockFile.lock");
    if (!lockFile.tryLock(100))
    {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText(QObject::tr("The application is already running.\nAllowed to run only one instance of the application."));
        msgBox.exec();
        return true;
    }
    return false;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QLockFile lockFile(QApplication::applicationDirPath() + "/LockFile.lock");
    if (ProcessExisted())
        return 0;
    DataManager manager;
    manager.load();
    Model m;
    m.showMaximized();
    return a.exec();
}
