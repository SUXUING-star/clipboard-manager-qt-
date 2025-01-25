// autostartmanager.h
#ifndef AUTOSTARTMANAGER_H
#define AUTOSTARTMANAGER_H

#include <QString>
#include <QObject>

class AutoStartManager : public QObject {
    Q_OBJECT
public:
    explicit AutoStartManager(QObject *parent = nullptr);
    bool isAutoStartEnabled() const;
    bool setAutoStart(bool enable);
    QString getError() const;

private:
    QString m_error;
    QString getRegistryPath() const;
    QString getExecutablePath() const;
    bool writeRegistry(bool enable);
};

#endif
