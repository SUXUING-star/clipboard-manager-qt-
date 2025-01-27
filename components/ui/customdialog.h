#ifndef CUSTOMDIALOG_H
#define CUSTOMDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>

class CustomDialog : public QDialog {
    Q_OBJECT

public:
    enum class DialogType {
        CloseConfirm,
        HistoryLimit
    };

    explicit CustomDialog(DialogType type, QWidget *parent = nullptr);

signals:
    void saveAndClose();
    void closeWithoutSave();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    QPoint dragPosition;
    bool isDragging = false;
    void initUI(DialogType type);
    void setupCloseConfirmDialog();
    void setupHistoryLimitDialog();
    QPushButton* createStyledButton(const QString& text, const QString& baseColor);
};

#endif // CUSTOMDIALOG_H
