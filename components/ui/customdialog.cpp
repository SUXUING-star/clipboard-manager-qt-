#include "customdialog.h"
#include <QMouseEvent>
#include <QApplication>
#include <QScreen>

CustomDialog::CustomDialog(DialogType type, QWidget *parent)
    : QDialog(parent, Qt::FramelessWindowHint)
{
    setAttribute(Qt::WA_DeleteOnClose);
    initUI(type);
}

void CustomDialog::initUI(DialogType type) {
    switch (type) {
    case DialogType::CloseConfirm:
        setupCloseConfirmDialog();
        break;
    case DialogType::HistoryLimit:
        setupHistoryLimitDialog();
        break;
    }
}

QPushButton* CustomDialog::createStyledButton(const QString& text, const QString& baseColor) {
    auto* button = new QPushButton(text);
    button->setFixedHeight(32);

    if (baseColor == "blue") {
        button->setStyleSheet(R"(
            QPushButton {
                background-color: #4B8BF4;
                color: white;
                border: none;
                border-radius: 4px;
                padding: 5px 15px;
                min-width: 80px;
                font-size: 13px;
            }
            QPushButton:hover {
                background-color: #357ABD;
            }
        )");
    } else if (baseColor == "red") {
        button->setStyleSheet(R"(
            QPushButton {
                background-color: #E34133;
                color: white;
                border: none;
                border-radius: 4px;
                padding: 5px 15px;
                min-width: 80px;
                font-size: 13px;
            }
            QPushButton:hover {
                background-color: #D03126;
            }
        )");
    } else {
        button->setStyleSheet(R"(
            QPushButton {
                background-color: #F5F5F5;
                color: #333333;
                border: 1px solid #DDDDDD;
                border-radius: 4px;
                padding: 5px 15px;
                min-width: 80px;
                font-size: 13px;
            }
            QPushButton:hover {
                background-color: #EBEBEB;
            }
        )");
    }
    return button;
}



void CustomDialog::setupCloseConfirmDialog() {
    // 设置整体对话框样式
    setStyleSheet(R"(
        QDialog {
            background-color: white;
            border: 1px solid #CCCCCC;
            border-radius: 8px;
        }
        QPushButton {
            height: 32px;
            min-width: 90px;
            border-radius: 4px;
            font-size: 13px;
            font-family: "Microsoft YaHei";
        }
        QPushButton#saveButton {
            background-color: #4B8BF4;
            color: white;
            border: none;
        }
        QPushButton#saveButton:hover {
            background-color: #357ABD;
        }
        QPushButton#noSaveButton {
            background-color: #E34133;
            color: white;
            border: none;
        }
        QPushButton#noSaveButton:hover {
            background-color: #D03126;
        }
        QPushButton#cancelButton {
            background-color: #F5F5F5;
            color: #333333;
            border: 1px solid #DDDDDD;
        }
        QPushButton#cancelButton:hover {
            background-color: #EBEBEB;
        }
        QLabel {
            color: #333333;
        }
    )");

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // 标题栏
    auto* titleBar = new QWidget;
    titleBar->setFixedHeight(45);
    titleBar->setStyleSheet("background: #4B8BF4; border-top-left-radius: 8px; border-top-right-radius: 8px;");

    auto* titleLayout = new QHBoxLayout(titleBar);
    titleLayout->setContentsMargins(20, 0, 20, 0);

    auto* titleLabel = new QLabel("关闭确认");
    titleLabel->setStyleSheet("color: white; font-size: 14px; font-weight: bold;");
    titleLayout->addWidget(titleLabel);

    mainLayout->addWidget(titleBar);

    // 内容区域
    auto* contentWidget = new QWidget;
    auto* contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setSpacing(20);
    contentLayout->setContentsMargins(20, 20, 20, 20);

    auto* messageLabel = new QLabel("是否要关闭剪贴板历史？\n当前记录将保存到本地。");
    messageLabel->setAlignment(Qt::AlignCenter);
    messageLabel->setStyleSheet("font-size: 13px; line-height: 1.5;");
    contentLayout->addWidget(messageLabel);

    // 按钮区域
    auto* buttonWidget = new QWidget;
    auto* buttonLayout = new QHBoxLayout(buttonWidget);
    buttonLayout->setSpacing(10);
    buttonLayout->setContentsMargins(0, 0, 0, 0);

    auto* saveButton = createStyledButton("保存并关闭", "blue");
    auto* noSaveButton = createStyledButton("直接关闭", "red");
    auto* cancelButton = createStyledButton("取消", "gray");

    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(noSaveButton);
    buttonLayout->addWidget(cancelButton);

    contentLayout->addWidget(buttonWidget);
    mainLayout->addWidget(contentWidget);

    connect(saveButton, &QPushButton::clicked, this, [this]() {
        emit saveAndClose();
        accept();
    });

    connect(noSaveButton, &QPushButton::clicked, this, [this]() {
        emit closeWithoutSave();
        accept();
    });

    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    setFixedSize(360, 180);
}
void CustomDialog::setupHistoryLimitDialog() {
    setStyleSheet(R"(
        QDialog {
            background: white;
            border-radius: 8px;
            border: 1px solid #CCCCCC;
        }
        QLabel {
            color: #333333;
        }
    )");

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // 标题栏
    auto* titleBar = new QWidget;
    titleBar->setFixedHeight(45);
    titleBar->setStyleSheet("background: #4B8BF4; border-top-left-radius: 8px; border-top-right-radius: 8px;");

    auto* titleLayout = new QHBoxLayout(titleBar);
    titleLayout->setContentsMargins(20, 0, 20, 0);

    auto* titleLabel = new QLabel("历史记录数量提醒");
    titleLabel->setStyleSheet("color: white; font-size: 14px; font-weight: bold;");
    titleLayout->addWidget(titleLabel);

    mainLayout->addWidget(titleBar);

    // 内容区域
    auto* contentWidget = new QWidget;
    contentWidget->setStyleSheet("background: transparent;");
    auto* contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setSpacing(20);
    contentLayout->setContentsMargins(20, 20, 20, 20);

    auto* messageLabel = new QLabel("历史记录数量已超过100条，\n较早的记录将会被自动清除。");
    messageLabel->setAlignment(Qt::AlignCenter);
    messageLabel->setStyleSheet("font-size: 13px; line-height: 1.5;");
    contentLayout->addWidget(messageLabel);

    auto* okButton = createStyledButton("确定", "blue");
    contentLayout->addWidget(okButton, 0, Qt::AlignCenter);

    mainLayout->addWidget(contentWidget);

    connect(okButton, &QPushButton::clicked, this, &QDialog::close);

    setFixedSize(300, 180);
}

void CustomDialog::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        isDragging = true;
        dragPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}

void CustomDialog::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton && isDragging) {
        move(event->globalPos() - dragPosition);
        event->accept();
    }
}

void CustomDialog::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        isDragging = false;
    }
}
