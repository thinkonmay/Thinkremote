#ifndef QEVENTLOGGER_H
#define QEVENTLOGGER_H

#include <QObject>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QDateTime>
#include <QTime>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QHash>
#include <QWidget>
#include <QDebug>
#include <QPixmap>

#define NONE -1
#define MOUSE 0
#define KEYBOARD 1
#define HOVER 2
#define FOCUS 3

class QEventLogger : public QObject {
    Q_OBJECT

public:
    explicit QEventLogger(const QString & logFileBaseName, QWidget * mainWidget, bool screenshotsEnabled, QObject * parent = 0);

protected:
    bool eventFilter(QObject * obj, QEvent * event);
    void appendToLog(const QString & inputType, const QString & eventType, const QString & targetWidget, const QString & details);

private:
    bool screenshotsEnabled;
    QString screenshotDirName;
    QWidget * mainWidget;
    QFile * logFile;
    QTextStream * log;
    QTime * time;
    QHash<QString, QHash<QObject *, uint> > widgetPointerToID;
};

#endif
