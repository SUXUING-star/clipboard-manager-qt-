#include "animationmanager.h"

AnimationManager::AnimationManager(QObject *parent) : QObject(parent) {}

void AnimationManager::playToastAnimation(QWidget* toast, const QPoint& centerPos) {
    // Enable widget caching
    toast->setAttribute(Qt::WA_TranslucentBackground);
    toast->setAttribute(Qt::WA_OpaquePaintEvent);

    auto* opacity = new QGraphicsOpacityEffect(toast);
    toast->setGraphicsEffect(opacity);

    auto* group = new QSequentialAnimationGroup(toast);

    // Optimize animation durations
    const int fadeTime = 200;
    const int pauseTime = 1000;

    // Create single combined animation
    QRect startGeom = toast->geometry();
    startGeom.moveCenter(centerPos + QPoint(0, 30)); // Reduced movement distance
    QRect endGeom = startGeom;
    endGeom.moveCenter(centerPos);

    // Combine position and opacity animations
    auto* parallelIn = new QParallelAnimationGroup;

    auto* moveIn = new QPropertyAnimation(toast, "geometry");
    moveIn->setDuration(fadeTime);
    moveIn->setStartValue(startGeom);
    moveIn->setEndValue(endGeom);
    moveIn->setEasingCurve(QEasingCurve::OutCubic); // Simplified easing

    auto* fadeIn = new QPropertyAnimation(opacity, "opacity");
    fadeIn->setDuration(fadeTime);
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);

    parallelIn->addAnimation(moveIn);
    parallelIn->addAnimation(fadeIn);

    // Simple pause
    auto* pause = new QPropertyAnimation(toast, "geometry");
    pause->setDuration(pauseTime);
    pause->setStartValue(endGeom);
    pause->setEndValue(endGeom);

    // Optimize exit animation
    auto* parallelOut = new QParallelAnimationGroup;

    auto* moveOut = new QPropertyAnimation(toast, "geometry");
    moveOut->setDuration(fadeTime);
    moveOut->setStartValue(endGeom);
    QRect finalGeom = endGeom;
    finalGeom.moveCenter(centerPos + QPoint(0, 30));
    moveOut->setEndValue(finalGeom);
    moveOut->setEasingCurve(QEasingCurve::InCubic);

    auto* fadeOut = new QPropertyAnimation(opacity, "opacity");
    fadeOut->setDuration(fadeTime);
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.0);

    parallelOut->addAnimation(moveOut);
    parallelOut->addAnimation(fadeOut);

    group->addAnimation(parallelIn);
    group->addAnimation(pause);
    group->addAnimation(parallelOut);

    connect(group, &QSequentialAnimationGroup::finished, toast, &QWidget::deleteLater);
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void AnimationManager::playPreviewShowAnimation(QWidget* preview, const QRect& finalGeometry) {
    preview->setAttribute(Qt::WA_TranslucentBackground);
    preview->setAttribute(Qt::WA_OpaquePaintEvent);

    QRect startGeom = finalGeometry;
    startGeom.setSize(QSize(finalGeometry.width()/2, finalGeometry.height()/2));
    startGeom.moveCenter(finalGeometry.center());

    auto* anim = new QPropertyAnimation(preview, "geometry");
    anim->setDuration(200); // Reduced duration
    anim->setStartValue(startGeom);
    anim->setEndValue(finalGeometry);
    anim->setEasingCurve(QEasingCurve::OutQuad);
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
    button->setAttribute(Qt::WA_TranslucentBackground);
    button->setAttribute(Qt::WA_OpaquePaintEvent);

    auto* scaleAnim = new QPropertyAnimation(button, "geometry");
    scaleAnim->setDuration(40);
    QRect geom = button->geometry();
    QRect smallerGeom = geom;
    smallerGeom.adjust(1, 1, -1, -1);

    scaleAnim->setStartValue(geom);
    scaleAnim->setEndValue(smallerGeom);
    scaleAnim->setEasingCurve(QEasingCurve::Linear);

    auto* reverseAnim = new QPropertyAnimation(button, "geometry");
    reverseAnim->setDuration(40);
    reverseAnim->setStartValue(smallerGeom);
    reverseAnim->setEndValue(geom);
    reverseAnim->setEasingCurve(QEasingCurve::Linear);

    auto* sequence = new QSequentialAnimationGroup;
    sequence->addAnimation(scaleAnim);
    sequence->addAnimation(reverseAnim);
    sequence->start(QAbstractAnimation::DeleteWhenStopped);
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
