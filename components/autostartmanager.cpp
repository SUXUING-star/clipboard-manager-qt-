// autostartmanager.cpp
#include "autostartmanager.h"
#include <QSettings>
#include <QCoreApplication>
#include <QFileInfo>
#include <QDir>


#ifdef Q_OS_WIN
#include <windows.h>
#endif



AutoStartManager::AutoStartManager(QObject *parent)
    : QObject(parent) {
}
QString AutoStartManager::getRegistryPath() const {
    return "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run";
}

QString AutoStartManager::getExecutablePath() const {
    QString appPath = QCoreApplication::applicationFilePath();
#ifdef Q_OS_WIN
    wchar_t shortPath[MAX_PATH];
    if(GetShortPathNameW(reinterpret_cast<const wchar_t*>(appPath.utf16()), shortPath, MAX_PATH)) {
        appPath = QString::fromWCharArray(shortPath);
    }
#endif
    return QDir::toNativeSeparators(appPath);
}

bool AutoStartManager::isAutoStartEnabled() const {
    QSettings settings(getRegistryPath(), QSettings::NativeFormat);
    QString regValue = settings.value("ClipboardManager").toString();
    return regValue == getExecutablePath();
}

bool AutoStartManager::writeRegistry(bool enable) {
    QSettings settings(getRegistryPath(), QSettings::NativeFormat);

    // 检查注册表权限
    if(!settings.isWritable()) {
        m_error = "No write permission to registry";
        return false;
    }

    try {
        if (enable) {
            QString path = QString("\"%1\"").arg(getExecutablePath()); // 处理路径中的空格
            settings.setValue("ClipboardManager", path);
        } else {
            settings.remove("ClipboardManager");
        }

        settings.sync();

        // 验证写入是否成功
        if(enable) {
            QString actual = settings.value("ClipboardManager").toString();
            if(actual != QString("\"%1\"").arg(getExecutablePath())) {
                m_error = "Registry write verification failed";
                return false;
            }
        }

        return settings.status() == QSettings::NoError;
    } catch (const std::exception& e) {
        m_error = QString("Registry operation failed: %1").arg(e.what());
        return false;
    }
}

bool AutoStartManager::setAutoStart(bool enable) {
    QString exePath = getExecutablePath();
    if (!QFileInfo::exists(exePath)) {
        m_error = QString("Executable not found: %1").arg(exePath);
        return false;
    }

    if (!QFileInfo(exePath).isExecutable()) {
        m_error = "File is not executable";
        return false;
    }

    return writeRegistry(enable);
}
QString AutoStartManager::getError() const {
    return m_error;
}
