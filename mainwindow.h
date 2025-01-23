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
#include "animationmanager.h"
#include "autostartmanager.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
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

    QLabel *titleLabel;
    QButtonGroup *categoryBtns;
    QCheckBox *autoStart;
    QPushButton *clearBtn;
    QWidget *contentArea;
    QListWidget *contentList;
    QClipboard *clipboard;

    QList<ClipboardItem> items;
    QDialog* createStyledDialog(const ClipboardItem& item);

private slots:
    void clipboardChanged();
    void showPreview(QListWidgetItem *item);
    void onCategoryChanged(QAbstractButton *button);
    void clearHistory();
    void setAutoStart(bool enable);
};

#endif
