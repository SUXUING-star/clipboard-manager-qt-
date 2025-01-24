#include "mainwindow.h"
#include "animationmanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include <QDateTime>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QParallelAnimationGroup>
#include <QGraphicsOpacityEffect>
#include <QMimeData>
#include <QClipboard>
#include <QMenu>
#include <QDialog>
#include <QTextEdit>
#include <QApplication>
#include <QDir>
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QBuffer>
#include <QScreen>
#include <QTimer>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("剪贴板历史");
    animationManager = new AnimationManager(this);
    setupUI();
    clipboard = QApplication::clipboard();
    connect(clipboard, &QClipboard::dataChanged, this, &MainWindow::clipboardChanged);

    autoStartManager = new AutoStartManager(this);
    loadAutoStartStatus();

    qDebug() << "MainWindow constructor end";
}

void MainWindow::setupUI() {
    qDebug() << "SetupUI start";

    auto *centralWidget = new QWidget;
    qDebug() << "Central widget created";

    auto *layout = new QHBoxLayout(centralWidget);

    layout->setSpacing(0);
    layout->setContentsMargins(0,0,0,0);

    // Left Panel
    leftPanel = new QWidget;
    qDebug() << "Left panel created";
    leftPanel->setFixedWidth(200);
    leftPanel->setStyleSheet("background-color: #4B8BF4;");

    qDebug() << "Left panel styled";

    auto *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setSpacing(0);
    leftLayout->setContentsMargins(0,0,0,0);

    auto *titleContainer = new QWidget;
    auto *titleLayout = new QVBoxLayout(titleContainer);
    titleLayout->setSpacing(8);
    titleLayout->setContentsMargins(20, 20, 20, 10);

    titleLabel = new QLabel("剪贴板历史");
    titleLabel->setStyleSheet("color: white; font-size: 16px; font-weight: bold;");
    titleLabel->setAlignment(Qt::AlignCenter);

    auto *iconLabel = new QLabel;
    QPixmap icon(":/icons/icon.png");
    iconLabel->setPixmap(icon.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setStyleSheet("margin-bottom: 10px;");

    titleLayout->addWidget(iconLabel);
    titleLayout->addWidget(titleLabel);
    leftLayout->addWidget(titleContainer);

    // Buttons
    categoryBtns = new QButtonGroup(this);

    struct BtnInfo {
        QString name;
        QString icon;
    };

    QVector<BtnInfo> buttons = {
        {"全部", ":/icons/startup_icon.svg"},
        {"文本", ":/icons/text_icon.svg"},
        {"图片", ":/icons/image_icon.svg"}
    };

    for(const auto &btn : buttons) {
        auto *button = new QPushButton(btn.name);
        button->setIcon(QIcon(btn.icon));
        button->setCheckable(true);
        button->setStyleSheet(R"(
            QPushButton {
                color: white;
                border: none;
                text-align: left;
                padding: 15px 20px;
                font-size: 14px;
            }
            QPushButton:hover {
                background-color: rgba(255,255,255,0.1);
            }
            QPushButton:checked {
                background-color: rgba(255,255,255,0.2);
            }
        )");
        categoryBtns->addButton(button);
        leftLayout->addWidget(button);
    }

    // Bottom controls
    autoStart = new QCheckBox("开机自启");
    autoStart->setStyleSheet("QCheckBox {color: white; padding: 10px 20px;}");
    leftLayout->addWidget(autoStart);

    clearBtn = new QPushButton("清空历史");
    clearBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #E34133;
            color: white;
            border: none;
            padding: 10px;
            margin: 10px 20px;
            border-radius: 4px;
        }
        QPushButton:hover {
            background-color: #D03126;
        }
    )");

    leftLayout->addWidget(clearBtn);

    // Content Area
    contentArea = new QWidget;
    auto *contentLayout = new QVBoxLayout(contentArea);
    contentLayout->setContentsMargins(20,20,20,20);

    contentList = new QListWidget;
    contentList->setStyleSheet(R"(
        QListWidget {
            background-color: white;
            border: none;
        }
        QListWidget::item {
            background-color: #F8F9FA;
            border-radius: 6px;
            margin-bottom: 10px;
        }
        QListWidget::item:hover {
            background-color: #E9ECEF;
        }
        QListWidget::item:selected {
            background-color: #E2E6EA;
        }
    )");
    // 添加事件过滤器处理窗口状态变化
    installEventFilter(this);

    contentLayout->addWidget(contentList);

    // Add to main layout
    layout->addWidget(leftPanel);
    layout->addWidget(contentArea);

    setCentralWidget(centralWidget);
    resize(800, 600);
    qDebug() << "main layout";

    // Connections
    connect(categoryBtns, &QButtonGroup::buttonClicked, this, &MainWindow::onCategoryChanged);
    connect(contentList, &QListWidget::itemDoubleClicked, this, &MainWindow::showPreview);
    connect(clearBtn, &QPushButton::clicked, this, &MainWindow::clearHistory);
    connect(autoStart, &QCheckBox::toggled, this, &MainWindow::setAutoStart);

    // Set default category
    if(categoryBtns->buttons().size() > 0) {
        categoryBtns->buttons().first()->setChecked(true);
    }
    qDebug() << "animation before setup";
    setupButtonAnimations();
    qDebug() << "animation setup";

}

void MainWindow::setupButtonAnimations() {
    // 为分类按钮添加动画
    for(auto *btn : categoryBtns->buttons()) {
        if(auto *pushBtn = qobject_cast<QPushButton*>(btn)) {
            connect(pushBtn, &QPushButton::clicked, [this, pushBtn]() {
                animationManager->playButtonClickAnimation(pushBtn);
            });
        }
    }

    // 为清空按钮添加动画
    connect(clearBtn, &QPushButton::clicked, [this]() {
        animationManager->playButtonClickAnimation(clearBtn);
    });
}

void MainWindow::clipboardChanged() {
    const QMimeData *mimeData = clipboard->mimeData();
    if(!mimeData) return;

    if(mimeData->hasImage()) {
        QPixmap image;
        try {
            image = qvariant_cast<QPixmap>(mimeData->imageData());
            if(image.isNull()) return;
        } catch (const std::exception& e) {
            qDebug() << "Error loading image from clipboard: " << e.what();
            return;
        }

        QString newHash = getImageHash(image);
        if(!items.isEmpty() && items.first().type == ClipboardItem::Image) {
            QString oldHash = getImageHash(items.first().image);
            if(newHash == oldHash) return;
        }

        ClipboardItem item;
        item.type = ClipboardItem::Image;
        item.image = image;
        item.timestamp = QDateTime::currentDateTime();
        items.prepend(item);
        updateList();
    } else if(mimeData->hasText()) {
        if(!items.isEmpty() && items.first().type == ClipboardItem::Text &&
            items.first().text == mimeData->text()) {
            return;
        }
        ClipboardItem item;
        item.type = ClipboardItem::Text;
        item.text = mimeData->text();
        item.timestamp = QDateTime::currentDateTime();
        items.prepend(item);
        updateList();
    }
}

QString MainWindow::getImageHash(const QPixmap& image) {
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    return QString(QCryptographicHash::hash(byteArray, QCryptographicHash::Md5).toHex());
}


// mainwindow.cpp - Add new dialog creation method
QDialog* MainWindow::createStyledDialog(const ClipboardItem& item) {
    QDialog* dialog = new QDialog(this, Qt::FramelessWindowHint);
    dialog->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    dialog->setAttribute(Qt::WA_TranslucentBackground);
    dialog->setModal(true);

    // 设置布局
    QVBoxLayout* layout = new QVBoxLayout(dialog);
    layout->setContentsMargins(0, 0, 0, 0);

    // 创建内容容器
    QWidget* container = new QWidget;
    container->setObjectName("container");
    container->setStyleSheet("#container { background:white; border-radius:8px; }");

    QVBoxLayout* containerLayout = new QVBoxLayout(container);
    containerLayout->setContentsMargins(0, 0, 0, 0);

    // 添加标题栏
    QWidget* titleBar = new QWidget;
    titleBar->setFixedHeight(50);
    titleBar->setStyleSheet("background:#4B8BF4; border-radius:8px 8px 0 0;");

    QHBoxLayout* titleLayout = new QHBoxLayout(titleBar);
    QLabel* titleLabel = new QLabel("预览");
    titleLabel->setStyleSheet("color:white; font-size:14px;");

    QPushButton* copyBtn = new QPushButton("复制");
    copyBtn->setStyleSheet(R"(
        QPushButton {
            background:rgba(255,255,255,0.2);
            color:white;
            border:none;
            padding:5px 15px;
            border-radius:4px;
        }
        QPushButton:hover {
            background:rgba(255,255,255,0.3);
        }
    )");

    QPushButton* closeBtn = new QPushButton("×");
    closeBtn->setStyleSheet(R"(
        QPushButton {
            color:white;
            border:none;
            font-size:20px;
            padding:5px 15px;
        }
        QPushButton:hover {
            background:rgba(255,255,255,0.1);
        }
    )");

    titleLayout->addWidget(titleLabel);
    titleLayout->addWidget(copyBtn);
    titleLayout->addWidget(closeBtn);
    containerLayout->addWidget(titleBar);

    // 添加内容
    if(item.type == ClipboardItem::Image) {
        QLabel* imageLabel = new QLabel;
        imageLabel->setPixmap(item.image.scaled(800, 600, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        imageLabel->setStyleSheet("background:white; padding:20px;");
        containerLayout->addWidget(imageLabel);
    } else {
        QTextEdit* textEdit = new QTextEdit;
        textEdit->setText(item.text);
        textEdit->setReadOnly(true);
        textEdit->setStyleSheet("background:white; padding:20px;");
        containerLayout->addWidget(textEdit);
    }

    // 添加阴影效果
    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect;
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 80));
    shadow->setOffset(0, 0);
    container->setGraphicsEffect(shadow);

    layout->addWidget(container);

    // 连接信号槽
    connect(copyBtn, &QPushButton::clicked, [this, item]() {
        if(item.type == ClipboardItem::Image) {
            clipboard->setPixmap(item.image);
        } else {
            clipboard->setText(item.text);
        }
        showToast("已复制到剪贴板");
    });

    connect(closeBtn, &QPushButton::clicked, [=]() {
        QRect geometry(0, 0, width() * 0.8, height() * 0.8);
        geometry.moveCenter(pos() + rect().center());
        animationManager->playPreviewHideAnimation(dialog, geometry);
    });

    return dialog;
}

void MainWindow::showPreview(QListWidgetItem *item) {
    int index = contentList->row(item);
    if(index < 0 || index >= items.size()) return;

    const auto &clipItem = items[index];
    QDialog* dialog = createStyledDialog(clipItem);
    dialog->show();

    QRect geometry(0, 0, width() * 0.8, height() * 0.8);
    geometry.moveCenter(pos() + rect().center());
    animationManager->playPreviewShowAnimation(dialog, geometry);
}

void MainWindow::onCategoryChanged(QAbstractButton *button) {
    updateList();
}

// 添加Toaster消息
void MainWindow::showToast(const QString &message) {
    auto *toast = new QDialog(this, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    toast->setAttribute(Qt::WA_DeleteOnClose);
    toast->setAttribute(Qt::WA_TranslucentBackground); // 添加透明背景支持

    auto *mainWidget = new QWidget(toast);
    mainWidget->setStyleSheet("background-color: #4B8BF4; border-radius: 4px;");

    auto *layout = new QVBoxLayout(toast);
    layout->setContentsMargins(0, 0, 0, 0);

    auto *innerLayout = new QVBoxLayout(mainWidget);
    auto *label = new QLabel(message);
    label->setStyleSheet("color: white; padding: 10px 20px; background: transparent;");
    innerLayout->addWidget(label);

    layout->addWidget(mainWidget);

    QPoint pos = this->geometry().center();
    toast->move(pos.x() - toast->width()/2, pos.y());
    toast->show();

    animationManager->playToastAnimation(toast, this->geometry().center());
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::WindowStateChange) {
        updateList(); // 强制刷新列表
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::updateList() {
    contentList->clear();
    QString category = categoryBtns->checkedButton()->text();

    contentList->setSpacing(10);
    contentList->setViewMode(QListView::ListMode);
    contentList->setVerticalScrollMode(QListWidget::ScrollPerPixel);
    contentList->setUniformItemSizes(false);
    contentList->setResizeMode(QListView::Adjust);

    for (const auto &item : items) {
        // 严格检查类型匹配
        bool shouldShow = false;
        if (category == "全部") {
            shouldShow = true;
        } else if (category == "文本" && item.type == ClipboardItem::Text) {
            shouldShow = true;
        } else if (category == "图片" && item.type == ClipboardItem::Image) {
            shouldShow = true;
        }

        if (shouldShow) {
            auto *container = new QWidget;
            container->setObjectName("itemContainer");
            container->setStyleSheet(R"(
                #itemContainer {
                    background-color: #F8F9FA;
                    border-radius: 6px;
                    margin: 5px;
                }
            )");

            auto *layout = new QHBoxLayout(container);
            layout->setContentsMargins(10, 10, 10, 10);
            layout->setSpacing(10);

            if (item.type == ClipboardItem::Image) {
                auto *imageLabel = new QLabel;
                imageLabel->setFixedSize(200, 150);
                imageLabel->setAlignment(Qt::AlignCenter);

                // 优化图片缩放质量
                if (!item.image.isNull()) {
                    // 保持宽高比并使用高质量缩放
                    QPixmap thumbnail = item.image.scaled(
                        imageLabel->size() * 2, // 2倍大小以提高清晰度
                        Qt::KeepAspectRatio,
                        Qt::SmoothTransformation
                        );
                    imageLabel->setPixmap(thumbnail.scaled(
                        imageLabel->size(),
                        Qt::KeepAspectRatio,
                        Qt::SmoothTransformation
                        ));
                } else {
                    imageLabel->setText("图片加载失败");
                }
                layout->addWidget(imageLabel);
            } else if (item.type == ClipboardItem::Text) {
                auto *textLabel = new QLabel(item.text.length() > 50
                                                 ? item.text.left(50) + "..."
                                                 : item.text);
                textLabel->setWordWrap(true);
                textLabel->setMinimumWidth(200);
                textLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
                layout->addWidget(textLabel);
            }

            layout->addStretch();

            auto *copyBtn = new QPushButton("复制");
            copyBtn->setFixedSize(60, 30);
            copyBtn->setStyleSheet(R"(
                QPushButton {
                    background: #4B8BF4;
                    color: white;
                    border: none;
                    border-radius: 4px;
                }
                QPushButton:hover {
                    background: #3579E3;
                }
            )");

            connect(copyBtn, &QPushButton::clicked, [this, item]() {
                if (item.type == ClipboardItem::Image) {
                    clipboard->setPixmap(item.image);
                } else {
                    clipboard->setText(item.text);
                }
                showToast("已复制到剪贴板");
            });
            layout->addWidget(copyBtn);

            auto *listItem = new QListWidgetItem;
            int itemHeight = (item.type == ClipboardItem::Image) ? 170 : 80;
            container->setFixedHeight(itemHeight);
            listItem->setSizeHint(QSize(contentList->viewport()->width(), itemHeight));

            contentList->addItem(listItem);
            contentList->setItemWidget(listItem, container);
        }
    }

    contentList->update();
}

void MainWindow::clearHistory() {
    items.clear();
    contentList->clear();
    showToast("历史记录已清空"); // 显示清空历史记录的Toast消息
}

void MainWindow::setAutoStart(bool enable) {
    if (autoStartManager->setAutoStart(enable)) {
        showToast(enable ? "已开启开机自启" : "已关闭开机自启");
    } else {
        showToast("设置开机自启失败: " + autoStartManager->getError());
    }
}

void MainWindow::loadAutoStartStatus() {
    autoStart->setChecked(autoStartManager->isAutoStartEnabled());
}

// MainWindow类添加重绘事件处理
void MainWindow::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);
    contentList->update();
    leftPanel->update();
}
