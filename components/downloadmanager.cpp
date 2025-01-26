#include "downloadmanager.h"
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QTextStream>
#include <QDateTime>
#include <QBuffer>

struct DownloadManager::DownloadManagerPrivate {
    QString lastError;
    std::unique_ptr<QBuffer> imageBuffer;
    std::unique_ptr<QBuffer> textBuffer;
};

DownloadManager::DownloadManager(QObject *parent)
    : QObject(parent)
    , d(std::make_unique<DownloadManagerPrivate>()) {}

DownloadManager::~DownloadManager() {
    clearBuffers();
}

void DownloadManager::clearBuffers() {
    if(d->imageBuffer) {
        d->imageBuffer->close();
        d->imageBuffer.reset();
    }
    if(d->textBuffer) {
        d->textBuffer->close();
        d->textBuffer.reset();
    }
}

bool DownloadManager::saveText(const QString& text, const QString& filePath) {
    if (!ensureDirectoryExists(filePath)) {
        return false;
    }

    clearBuffers();
    d->textBuffer = std::make_unique<QBuffer>();

    if (!d->textBuffer->open(QIODevice::WriteOnly)) {
        d->lastError = "无法初始化缓冲区";
        return false;
    }

    QTextStream stream(d->textBuffer.get());
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    stream.setCodec("UTF-8");
#endif
    stream << text;
    stream.flush();

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        d->lastError = "无法创建文件: " + file.errorString();
        return false;
    }

    file.write(d->textBuffer->data());
    file.close();
    clearBuffers();
    return true;
}

bool DownloadManager::saveImage(const QPixmap& image, const QString& filePath) {
    if (!ensureDirectoryExists(filePath)) {
        return false;
    }

    clearBuffers();
    d->imageBuffer = std::make_unique<QBuffer>();

    if (!d->imageBuffer->open(QIODevice::WriteOnly)) {
        d->lastError = "无法初始化图片缓冲区";
        return false;
    }

    if (!image.save(d->imageBuffer.get(), "PNG")) {
        d->lastError = "无法转换图片数据";
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        d->lastError = "无法保存图片";
        return false;
    }

    file.write(d->imageBuffer->data());
    file.close();
    clearBuffers();
    return true;
}

bool DownloadManager::ensureDirectoryExists(const QString& filePath) {
    QFileInfo fileInfo(filePath);
    QDir dir = fileInfo.absoluteDir();

    if (!dir.exists() && !dir.mkpath(".")) {
        d->lastError = "无法创建目录";
        return false;
    }
    return true;
}

QString DownloadManager::getLastError() const {
    return d->lastError;
}
