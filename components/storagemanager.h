// storagemanager.h
#ifndef STORAGEMANAGER_H
#define STORAGEMANAGER_H

#include <QObject>
#include <QPixmap>
#include <QDateTime>
#include <QList>
#include <memory>
#include <QCache>


class StorageManager : public QObject {
    Q_OBJECT
public:
    struct ClipboardData {
        enum Type { Text, Image } type;
        QString text;
        QPixmap image;
        QDateTime timestamp;
        QString hash;  // For images only
    };

    explicit StorageManager(QObject *parent = nullptr);
    ~StorageManager();

    // 禁用拷贝构造和赋值操作
    StorageManager(const StorageManager&) = delete;
    StorageManager& operator=(const StorageManager&) = delete;

    bool saveHistory(const QList<ClipboardData>& items);
    QList<ClipboardData> loadHistory();
    QString getLastError() const;
    void setStorageLimit(int maxItems) { m_maxItems = maxItems; }
    void setCacheSize(int megabytes);
    void clearCache();

private:
    struct Private;
    std::unique_ptr<Private> d;  // PIMPL模式

    QString getStoragePath() const;
    QString getImagesPath() const;
    bool ensureDirectoryExists(const QString& path) const;
    bool saveImage(const QPixmap& image, const QString& hash) const;
    QPixmap loadImage(const QString& hash) const;
    void initializeCache();

    // 缓存相关
    static const int DEFAULT_CACHE_SIZE = 50; // MB
    QCache<QString, QPixmap> m_imageCache;
    int m_maxItems = 100;
};

#endif // STORAGEMANAGER_H
