#include "animationmanager.h"
#include <QGraphicsScale>
#include <QPauseAnimation>

AnimationManager::AnimationManager(QObject *parent) : QObject(parent) {}

void AnimationManager::playToastAnimation(QWidget* toast, const QPoint& centerPos) {
    toast->setAttribute(Qt::WA_DeleteOnClose);
    toast->setAttribute(Qt::WA_TranslucentBackground);

    auto* opacity = new QGraphicsOpacityEffect(toast);
    toast->setGraphicsEffect(opacity);

    auto* group = new QSequentialAnimationGroup(toast);
    activeAnimations.append(group);

    const int fadeTime = 150; // 减少动画时间
    const int pauseTime = 800;

    QRect startGeom = toast->geometry();
    startGeom.moveCenter(centerPos + QPoint(0, 20));
    QRect endGeom = startGeom;
    endGeom.moveCenter(centerPos);

    auto* parallelIn = new QParallelAnimationGroup;

    auto* moveIn = new QPropertyAnimation(toast, "geometry");
    moveIn->setDuration(fadeTime);
    moveIn->setStartValue(startGeom);
    moveIn->setEndValue(endGeom);
    moveIn->setEasingCurve(QEasingCurve::OutQuad);

    auto* fadeIn = new QPropertyAnimation(opacity, "opacity");
    fadeIn->setDuration(fadeTime);
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);

    parallelIn->addAnimation(moveIn);
    parallelIn->addAnimation(fadeIn);

    auto* pause = new QPauseAnimation(pauseTime);

    auto* parallelOut = new QParallelAnimationGroup;

    auto* moveOut = new QPropertyAnimation(toast, "geometry");
    moveOut->setDuration(fadeTime);
    moveOut->setStartValue(endGeom);
    QRect finalGeom = endGeom;
    finalGeom.moveCenter(centerPos + QPoint(0, -20));
    moveOut->setEndValue(finalGeom);
    moveOut->setEasingCurve(QEasingCurve::InQuad);

    auto* fadeOut = new QPropertyAnimation(opacity, "opacity");
    fadeOut->setDuration(fadeTime);
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.0);

    parallelOut->addAnimation(moveOut);
    parallelOut->addAnimation(fadeOut);

    group->addAnimation(parallelIn);
    group->addAnimation(pause);
    group->addAnimation(parallelOut);

    connect(group, &QSequentialAnimationGroup::finished, this, [this, group, toast]() {
        toast->close();
        cleanupAnimation(group);
    });

    group->start(QAbstractAnimation::DeleteWhenStopped);
}



AnimationManager::~AnimationManager() {
    qDeleteAll(activeAnimations);
}


void AnimationManager::playPreviewShowAnimation(QWidget* preview, const QRect& finalGeometry) {
    if(!preview || !preview->isVisible()) return;

    preview->setAttribute(Qt::WA_TranslucentBackground);

    QRect startGeom = finalGeometry;
    startGeom.setSize(QSize(finalGeometry.width()/2, finalGeometry.height()/2));
    startGeom.moveCenter(finalGeometry.center());

    auto* anim = new QPropertyAnimation(preview, "geometry");
    activeAnimations.append(anim);

    anim->setDuration(200);
    anim->setStartValue(startGeom);
    anim->setEndValue(finalGeometry);
    anim->setEasingCurve(QEasingCurve::OutQuad);

    connect(anim, &QPropertyAnimation::finished, this, [this, anim]() {
        cleanupAnimation(anim);
    });

    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void AnimationManager::playPreviewHideAnimation(QWidget* preview, const QRect& finalGeometry) {
    QRect startGeom = preview->geometry();
    QRect endGeom = startGeom;
    endGeom.setSize(QSize(startGeom.width()/2, startGeom.height()/2));
    endGeom.moveCenter(startGeom.center());

    auto* anim = new QPropertyAnimation(preview, "geometry");
    anim->setDuration(150); // Reduced duration
    anim->setStartValue(startGeom);
    anim->setEndValue(endGeom);
    anim->setEasingCurve(QEasingCurve::InQuad);

    connect(anim, &QPropertyAnimation::finished, preview, &QWidget::close);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void AnimationManager::playListItemAnimation(QWidget* item) {
    item->setAttribute(Qt::WA_TranslucentBackground);
    item->setAttribute(Qt::WA_OpaquePaintEvent);

    auto* heightAnim = new QPropertyAnimation(item, "minimumHeight");
    heightAnim->setDuration(150);
    heightAnim->setStartValue(0);
    heightAnim->setEndValue(item->sizeHint().height());
    heightAnim->setEasingCurve(QEasingCurve::OutQuad);
    heightAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

void AnimationManager::playButtonClickAnimation(QPushButton* button) {
    if(!button) return;

    // 停止已有动画
    for(auto* anim : activeAnimations) {
        if(anim->parent() == button) {
            cleanupAnimation(anim);
            return;  // 等待动画完全停止
        }
    }

    // 使用属性存储原始大小和状态
    static const char* ORIGINAL_GEOMETRY = "originalGeometry";
    static const char* IS_SCALED = "isScaled";

    if(!button->property(ORIGINAL_GEOMETRY).isValid()) {
        button->setProperty(ORIGINAL_GEOMETRY, button->geometry());
        button->setProperty(IS_SCALED, false);
    }

    auto* animGroup = new QParallelAnimationGroup(button);
    activeAnimations.append(animGroup);

    QRect originalGeom = button->property(ORIGINAL_GEOMETRY).toRect();
    bool isScaled = button->property(IS_SCALED).toBool();

    QRect targetGeom;
    if(!isScaled) {
        targetGeom = originalGeom;
        targetGeom.adjust(
            originalGeom.width() * 0.025,
            originalGeom.height() * 0.025,
            -originalGeom.width() * 0.025,
            -originalGeom.height() * 0.025
            );
    } else {
        targetGeom = originalGeom;
    }

    auto* anim = new QPropertyAnimation(button, "geometry", animGroup);
    anim->setDuration(60);
    anim->setStartValue(button->geometry());
    anim->setEndValue(targetGeom);
    anim->setEasingCurve(!isScaled ? QEasingCurve::InQuad : QEasingCurve::OutQuad);

    connect(animGroup, &QParallelAnimationGroup::finished, this, [this, animGroup, button, isScaled]() {
        button->setProperty(IS_SCALED, !isScaled);
        cleanupAnimation(animGroup);
    });

    animGroup->start(QAbstractAnimation::DeleteWhenStopped);
}

QEasingCurve AnimationManager::getCustomBounceEasing() const {
    QEasingCurve curve(QEasingCurve::OutBounce);
    curve.setAmplitude(1.2);
    curve.setPeriod(0.3);
    return curve;
}

QEasingCurve AnimationManager::getCustomElasticEasing() const {
    QEasingCurve curve(QEasingCurve::OutElastic);
    curve.setAmplitude(1.1);
    curve.setPeriod(0.3);
    return curve;
}
