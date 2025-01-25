// animationmanager.h
#ifndef ANIMATIONMANAGER_H
#define ANIMATIONMANAGER_H

#include <QObject>
#include <QWidget>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QParallelAnimationGroup>
#include <QGraphicsOpacityEffect>
#include <QEasingCurve>
#include <QPushButton>

class AnimationManager : public QObject {
    Q_OBJECT

public:
    explicit AnimationManager(QObject *parent = nullptr);
    ~AnimationManager() override;

    // Toast动画
    void playToastAnimation(QWidget* toast, const QPoint& centerPos);

    // 预览窗口动画
    void playPreviewShowAnimation(QWidget* preview, const QRect& finalGeometry);
    void playPreviewHideAnimation(QWidget* preview, const QRect& finalGeometry);

    // 列表项目动画
    void playListItemAnimation(QWidget* item);

    // 按钮点击动画
    void playButtonClickAnimation(QPushButton* button);

    // 自定义缓动曲线
    QEasingCurve getCustomBounceEasing() const;
    QEasingCurve getCustomElasticEasing() const;

private:
    QGraphicsEffect *scaleEffect;
    QList<QAbstractAnimation*> activeAnimations; // 跟踪所有活动的动画

    void cleanupAnimation(QAbstractAnimation* animation){
        if(!animation) return;

        activeAnimations.removeOne(animation);
        animation->stop();
        animation->deleteLater();

        // Clear any orphaned effects
        QList<QGraphicsEffect*> effects = animation->findChildren<QGraphicsEffect*>();
        qDeleteAll(effects);
    };
    const int TOAST_DURATION = 300;
    const int PREVIEW_DURATION = 300;
    const int LISTITEM_DURATION = 300;
    const int BUTTON_DURATION = 150;
};

#endif // ANIMATIONMANAGER_H
