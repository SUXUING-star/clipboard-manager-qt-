#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <QObject>
#include <QPixmap>
#include <QString>
#include <memory>

class DownloadManager : public QObject {
    Q_OBJECT
public:
    explicit DownloadManager(QObject *parent = nullptr);
    ~DownloadManager();

    bool saveText(const QString& text, const QString& filePath);
    bool saveImage(const QPixmap& image, const QString& filePath);
    QString getLastError() const;

private:
    struct DownloadManagerPrivate;
    std::unique_ptr<DownloadManagerPrivate> d;
    bool ensureDirectoryExists(const QString& filePath);
    void clearBuffers();
};

#endif
