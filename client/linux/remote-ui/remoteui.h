#ifndef REMOTEUI_H
#define REMOTEUI_H

#include <QMainWindow>
#include <GEventLogger.h>
#include <../remote-app/include/remote-app.h>

QT_BEGIN_NAMESPACE
namespace Ui { class RemoteUI; }
QT_END_NAMESPACE

class RemoteUI : public QMainWindow
{
    Q_OBJECT

public:
    RemoteUI(QWidget *parent = nullptr);
    ~RemoteUI();
    QEventLogger* eventLogger;

private:
    Ui::RemoteUI *ui;
};
#endif // REMOTEUI_H
