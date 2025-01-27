// storagemanager.cpp
#include "storagemanager.h"
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QBuffer>
#include <QCryptographicHash>
#include <QMutex>
#include <QMutexLocker>

struct StorageManager::Private {
    QString lastError;
    QMutex mutex;
    std::unique_ptr<QBuffer> imageBuffer;
};

StorageManager::StorageManager(QObject *parent)
    : QObject(parent)
    , d(std::make_unique<Private>())
{
    ensureDirectoryExists(getStoragePath());
    ensureDirectoryExists(getImagesPath());
    initializeCache();
}

StorageManager::~StorageManager() {
    clearCache();
}

void StorageManager::initializeCache() {
    m_imageCache.setMaxCost(DEFAULT_CACHE_SIZE * 1024); // 转换为KB
}

void StorageManager::setCacheSize(int megabytes) {
    m_imageCache.setMaxCost(megabytes * 1024);
}

void StorageManager::clearCache() {
    m_imageCache.clear();
    if (d->imageBuffer) {
        d->imageBuffer->close();
        d->imageBuffer.reset();
    }
}

bool StorageManager::saveHistory(const QList<ClipboardData>& items) {
    QMutexLocker locker(&d->mutex); // 线程安全

    QJsonArray jsonArray;
    int count = 0;

    try {
        for (const auto& item : items) {
            if (count >= m_maxItems) break;

            QJsonObject itemObj;
            itemObj["type"] = (item.type == ClipboardData::Text) ? "text" : "image";
            itemObj["timestamp"] = item.timestamp.toString(Qt::ISODate);

            if (item.type == ClipboardData::Text) {
                itemObj["text"] = item.text;
            } else {
                itemObj["hash"] = item.hash;
                // 检查缓存中是否已有该图片
                if (!m_imageCache.contains(item.hash)) {
                    if (!saveImage(item.image, item.hash)) {
                        continue;
                    }
                    // 保存到缓存
                    auto* cachedImage = new QPixmap(item.image);
                    m_imageCache.insert(item.hash, cachedImage,
                                        (item.image.width() * item.image.height() * 4) / 1024); // 预估大小（KB）
                }
            }

            jsonArray.append(itemObj);
            count++;
        }

        QJsonDocument doc(jsonArray);
        QFile file(getStoragePath() + "/history.json");

        if (!file.open(QIODevice::WriteOnly)) {
            d->lastError = "无法打开历史记录文件进行写入";
            return false;
        }

        file.write(doc.toJson(QJsonDocument::Compact)); // 使用压缩格式
        return true;
    } catch (const std::exception& e) {
        d->lastError = QString("保存历史记录时发生错误: %1").arg(e.what());
        return false;
    }
}

QList<StorageManager::ClipboardData> StorageManager::loadHistory() {
    QMutexLocker locker(&d->mutex); // 线程安全

    QList<ClipboardData> items;
    QFile file(getStoragePath() + "/history.json");

    try {
        if (!file.open(QIODevice::ReadOnly)) {
            d->lastError = "无法打开历史记录文件";
            return items;
        }

        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        if (doc.isNull()) {
            d->lastError = "历史记录文件格式错误";
            return items;
        }

        QJsonArray array = doc.array();
        items.reserve(array.size()); // 预分配空间

        for (const auto& value : array) {
            QJsonObject obj = value.toObject();
            ClipboardData item;

            item.timestamp = QDateTime::fromString(obj["timestamp"].toString(), Qt::ISODate);
            if (obj["type"].toString() == "text") {
                item.type = ClipboardData::Text;
                item.text = obj["text"].toString();
            } else {
                item.type = ClipboardData::Image;
                item.hash = obj["hash"].toString();

                // 先从缓存加载
                if (auto* cachedImage = m_imageCache.object(item.hash)) {
                    item.image = *cachedImage;
                } else {
                    item.image = loadImage(item.hash);
                    if (!item.image.isNull()) {
                        // 添加到缓存
                        auto* newImage = new QPixmap(item.image);
                        m_imageCache.insert(item.hash, newImage,
                                            (item.image.width() * item.image.height() * 4) / 1024);
                    } else {
                        continue;
                    }
                }
            }

            items.append(std::move(item)); // 使用移动语义
        }

        return items;
    } catch (const std::exception& e) {
        d->lastError = QString("加载历史记录时发生错误: %1").arg(e.what());
        return items;
    }
}

bool StorageManager::saveImage(const QPixmap& image, const QString& hash) const {
    if (!d->imageBuffer) {
        d->imageBuffer = std::make_unique<QBuffer>();
    }

    d->imageBuffer->close();
    d->imageBuffer->open(QIODevice::WriteOnly);

    if (!image.save(d->imageBuffer.get(), "PNG", 75)) { // 使用压缩
        return false;
    }

    QFile file(getImagesPath() + "/" + hash + ".png");
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    file.write(d->imageBuffer->data());
    d->imageBuffer->close();
    return true;
}

QPixmap StorageManager::loadImage(const QString& hash) const {
    QPixmap image;
    QString path = getImagesPath() + "/" + hash + ".png";

    if (QFile::exists(path)) {
        image.load(path, "PNG");
    }
    return image;
}

QString StorageManager::getLastError() const {
    return d->lastError;
}

QString StorageManager::getStoragePath() const {
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
}

QString StorageManager::getImagesPath() const {
    return getStoragePath() + "/images";
}

bool StorageManager::ensureDirectoryExists(const QString& path) const {
    QDir dir(path);
    return dir.exists() || dir.mkpath(".");
}
