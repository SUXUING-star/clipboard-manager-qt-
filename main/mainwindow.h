#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QButtonGroup>
#include <QCheckBox>
#include <QPushButton>
#include <QListWidget>
#include <QClipboard>
#include <QSettings>
#include <QDateTime>
#include <QCryptographicHash>
#include <QBuffer> // 添加 QBuffer 的头文件
#include <QPixmap>
#include "../components/animationmanager.h"
#include "../components/autostartmanager.h"

#include <QHash>
#include <QString>
#include <QPixmap>

#include <memory>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override; // 添加析构函数声明

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    QLabel *descLabel;
    //动画管理类
    AnimationManager *animationManager;
    //自启动管理类
    AutoStartManager *autoStartManager;
    struct ClipboardItem {
        enum Type { Text, Image } type;
        QString text;
        QPixmap image;
        QDateTime timestamp;
    };


    void setupUI();
    void updateList();
    QDialog* createPreviewDialog(const ClipboardItem& item);
    QPushButton* createCopyButton();
    void copyToClipboard(const QString& text);
    void copyImageToClipboard(const QPixmap& image);
    void showToast(const QString &message);
    void loadAutoStartStatus();
    void setupButtonAnimations();

    // 缓存机制


    QLabel *titleLabel;
    QButtonGroup *categoryBtns;
    QCheckBox *autoStart;
    QPushButton *clearBtn;
    QWidget *contentArea;
    QWidget *leftPanel;
    QListWidget *contentList;
    QClipboard *clipboard;

    QString getImageHash(const QPixmap& image);
    QLabel* createImageLabel(const QPixmap& image);
    QLabel* createTextLabel(const QString& text);

    QList<ClipboardItem> items;
    QDialog* createStyledDialog(const ClipboardItem& item);

    // 添加缓存相关成员
    void clearCache();
    void addToCache(const QString& hash, const QPixmap& image);
    QPixmap getFromCache(const QString& hash);

    // 缓存相关成员
    QHash<QString, QPixmap> imageCache;
    const int MAX_CACHE_SIZE = 20; // MB
    int currentCacheSize = 0;




private slots:
    void clipboardChanged();
    void showPreview(QListWidgetItem *item);
    void onCategoryChanged(QAbstractButton *button);
    void clearHistory();
    void setAutoStart(bool enable);
};

#endif
