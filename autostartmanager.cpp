// autostartmanager.cpp
#include "autostartmanager.h"
#include <QSettings>
#include <QCoreApplication>
#include <QFileInfo>
#include <QDir>

AutoStartManager::AutoStartManager(QObject *parent)
    : QObject(parent) {
}

QString AutoStartManager::getRegistryPath() const {
    return "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run";
}

QString AutoStartManager::getExecutablePath() const {
    QString appPath = QCoreApplication::applicationFilePath();
    return QDir::toNativeSeparators(appPath);
}

bool AutoStartManager::isAutoStartEnabled() const {
    QSettings settings(getRegistryPath(), QSettings::NativeFormat);
    QString regValue = settings.value("ClipboardManager").toString();
    return regValue == getExecutablePath();
}

bool AutoStartManager::writeRegistry(bool enable) {
    try {
        QSettings settings(getRegistryPath(), QSettings::NativeFormat);

        if (enable) {
            settings.setValue("ClipboardManager", getExecutablePath());
        } else {
            settings.remove("ClipboardManager");
        }

        settings.sync();

        if (settings.status() != QSettings::NoError) {
            m_error = "Registry operation failed";
            return false;
        }

        return true;
    } catch (const std::exception& e) {
        m_error = QString("Exception: %1").arg(e.what());
        return false;
    }
}

bool AutoStartManager::setAutoStart(bool enable) {
    if (!QFileInfo::exists(getExecutablePath())) {
        m_error = "Executable not found";
        return false;
    }

    return writeRegistry(enable);
}

QString AutoStartManager::getError() const {
    return m_error;
}
