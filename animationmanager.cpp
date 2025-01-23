// animationmanager.cpp
#include "animationmanager.h"

AnimationManager::AnimationManager(QObject *parent) : QObject(parent) {}

void AnimationManager::playToastAnimation(QWidget* toast, const QPoint& centerPos) {
    auto* opacity = new QGraphicsOpacityEffect(toast);
    toast->setGraphicsEffect(opacity);

    // 创建动画序列
    auto* group = new QSequentialAnimationGroup(toast);

    // 上移渐入动画
    auto* moveIn = new QPropertyAnimation(toast, "geometry");
    moveIn->setDuration(TOAST_DURATION);
    QRect startGeom = toast->geometry();
    startGeom.moveCenter(centerPos + QPoint(0, 50));
    QRect endGeom = startGeom;
    endGeom.moveCenter(centerPos);
    moveIn->setStartValue(startGeom);
    moveIn->setEndValue(endGeom);
    moveIn->setEasingCurve(getCustomBounceEasing());

    auto* fadeIn = new QPropertyAnimation(opacity, "opacity");
    fadeIn->setDuration(TOAST_DURATION);
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);

    auto* parallelIn = new QParallelAnimationGroup;
    parallelIn->addAnimation(moveIn);
    parallelIn->addAnimation(fadeIn);

    // 停留动画
    auto* pause = new QPropertyAnimation(opacity, "opacity");
    pause->setDuration(1400);
    pause->setStartValue(1.0);
    pause->setEndValue(1.0);

    // 下移渐出动画
    auto* moveOut = new QPropertyAnimation(toast, "geometry");
    moveOut->setDuration(TOAST_DURATION);
    moveOut->setStartValue(endGeom);
    QRect finalGeom = endGeom;
    finalGeom.moveCenter(centerPos + QPoint(0, 50));
    moveOut->setEndValue(finalGeom);
    moveOut->setEasingCurve(QEasingCurve::InCubic);

    auto* fadeOut = new QPropertyAnimation(opacity, "opacity");
    fadeOut->setDuration(TOAST_DURATION);
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.0);

    auto* parallelOut = new QParallelAnimationGroup;
    parallelOut->addAnimation(moveOut);
    parallelOut->addAnimation(fadeOut);

    group->addAnimation(parallelIn);
    group->addAnimation(pause);
    group->addAnimation(parallelOut);

    connect(group, &QSequentialAnimationGroup::finished, toast, &QWidget::deleteLater);
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

// 在你的代码中，修改 PREVIEW_DURATION 的定义
#define PREVIEW_DURATION 250 // 尝试新的值

void AnimationManager::playPreviewShowAnimation(QWidget* preview, const QRect& finalGeometry) {
    QRect startGeom = finalGeometry;
    startGeom.setSize(QSize(10, 10)); // 设置一个较小的初始尺寸
    startGeom.moveCenter(finalGeometry.center());

    auto* anim = new QPropertyAnimation(preview, "geometry");
    anim->setDuration(PREVIEW_DURATION);
    anim->setStartValue(startGeom);
    anim->setEndValue(finalGeometry);
    anim->setEasingCurve(QEasingCurve::OutQuad); // 使用 OutQuad
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void AnimationManager::playPreviewHideAnimation(QWidget* preview, const QRect& finalGeometry) {
    QRect startGeom = preview->geometry();
    QRect endGeom = finalGeometry;
    endGeom.setSize(QSize(10, 10)); // 设置一个较小的结束尺寸
    endGeom.moveCenter(finalGeometry.center());

    auto* anim = new QPropertyAnimation(preview, "geometry");
    anim->setDuration(PREVIEW_DURATION);
    anim->setStartValue(startGeom);
    anim->setEndValue(endGeom);
    anim->setEasingCurve(QEasingCurve::OutQuad); // 使用 OutQuad

    connect(anim, &QPropertyAnimation::finished, preview, &QWidget::close);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}
void AnimationManager::playListItemAnimation(QWidget* item) {
    // 减少动画时长，避免视觉延迟
    const int ANIM_DURATION = 200;  
    // 使用缓存机制提升渲染性能
    item->setAttribute(Qt::WA_TranslucentBackground);
    item->setAttribute(Qt::WA_OpaquePaintEvent);

    auto* heightAnim = new QPropertyAnimation(item, "minimumHeight");
    heightAnim->setDuration(ANIM_DURATION);
    heightAnim->setStartValue(0);
    heightAnim->setEndValue(item->sizeHint().height());
    heightAnim->setEasingCurve(QEasingCurve::OutQuad); // 使用更轻量的缓动曲线
}

void AnimationManager::playButtonClickAnimation(QPushButton* button) {
	// Reduce animation duration for better responsiveness
	const int CLICK_DURATION = 50; // Reduced from default BUTTON_DURATION

	// Enable widget caching for smoother animation
	button->setAttribute(Qt::WA_TranslucentBackground);
	button->setAttribute(Qt::WA_OpaquePaintEvent);

	auto* scaleAnim = new QPropertyAnimation(button, "geometry");
	scaleAnim->setDuration(CLICK_DURATION);
	QRect geom = button->geometry();
	QRect smallerGeom = geom;
	smallerGeom.adjust(1, 1, -1, -1); // Smaller scaling for faster animation

	scaleAnim->setStartValue(geom);
	scaleAnim->setEndValue(smallerGeom);
	scaleAnim->setEasingCurve(QEasingCurve::OutQuad); // Simpler easing curve

	auto* reverseAnim = new QPropertyAnimation(button, "geometry");
	reverseAnim->setDuration(CLICK_DURATION);
	reverseAnim->setStartValue(smallerGeom);
	reverseAnim->setEndValue(geom);
	reverseAnim->setEasingCurve(QEasingCurve::InQuad); // Simpler easing curve

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
