/**
 * @file MyInformationBox.h
 *
 * @brief 浏览器风格的弹出通知框组件，会自动析构
 *
 * @version 1.2
 * @date 2025/1/20
 *
 * @author uxinghai
 * @copyright Copyright (c) 2025
 */

#pragma once

#include <QGraphicsDropShadowEffect>
#include <QLabel>
#include <QPropertyAnimation>
#include <QTimer>
#include <QWidget>
#include <utility>

class MyInformationBox final : public QWidget {
  Q_OBJECT

public:
  /**
   * @brief 构造函数
   * @param parent 父窗口
   * @param text 显示的文本内容
   * @param displayDuration 显示持续时间(毫秒)，默认3000ms
   * @param fadeTime 动画从初始状态到终止状态的时长，默认300ms
   */
  explicit MyInformationBox(QWidget* parent,
                            QString text = QString(),
                            const int displayDuration = 3000,
                            const int fadeTime = 300)
    : QWidget(parent),
      label(new QLabel(this)),
      anim(new QPropertyAnimation(this)),
      displayDuration(displayDuration),
      parent(parent),
      text(std::move(text))
  {
    // 设置无边框窗口
    this->setWindowFlags(
      Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->setContentsMargins(0, 0, 0, 0);

    // 设置标签样式
    label->setStyleSheet(
      "QLabel {"
      "   color: white;"
      "   background-color: rgba(0, 0, 0, 220);"
      "   border-radius: 8px;"
      "   padding: 12px 20px;"
      "   font: 14px 'Segoe UI', sans-serif;"
      "   font-weight: bold;"
      "}"
    );
    label->setText(text);
    label->adjustSize();

    // 添加阴影效果
    auto* shadowEffect = new QGraphicsDropShadowEffect(this);
    shadowEffect->setBlurRadius(15);
    shadowEffect->setOffset(2, 2);
    shadowEffect->setColor(QColor(0, 0, 0, 150));
    label->setGraphicsEffect(shadowEffect);

    // 设置窗口大小
    this->resize(label->size());

    anim->setDuration(fadeTime);
    anim->setEasingCurve(QEasingCurve::InOutQuad);
  }

  void show()
  {
    disconnect(anim, &QPropertyAnimation::finished,
               this, &MyInformationBox::close);

    const QRect parentGeometry = parentWidget()->geometry();

    // 当前窗口的宽度和高度
    const int windowWidth = this->width();
    const int windowHeight = this->height();

    // 计算窗口的起始和目标位置
    // 从顶部外部开始，稍微下滑
    QPoint startPos((parentGeometry.width() - windowWidth) / 2, -windowHeight);
    const QPoint endPos((parentGeometry.width() - windowWidth) / 2, 20);

    // 设置动画属性
    anim->setTargetObject(this);
    anim->setPropertyName("pos");///< 位置变换

    // 设置动画的起始和结束位置
    this->move(parentWidget()->mapToGlobal(startPos));
    anim->setStartValue(parentWidget()->mapToGlobal(startPos));
    anim->setEndValue(parentWidget()->mapToGlobal(endPos));

    // 显示窗口
    QWidget::show();
    anim->start();

    // 设置定时隐藏
    QTimer::singleShot(displayDuration, [this, startPos] {
      anim->setStartValue(this->pos());
      anim->setEndValue(
        parentWidget()->mapToGlobal(startPos));
      anim->start();

      // 动画完成后隐藏窗口
      connect(anim, &QPropertyAnimation::finished,
              this, &MyInformationBox::close);
    });
  }

  void setText(const QString& text)
  {
    this->text = text;
    label->setText(text);
    label->adjustSize();
    this->resize(label->size());
  }

private:
  QLabel* label;
  QPropertyAnimation* anim;
  int displayDuration; ///< 显示时长 超出隐藏
  QWidget* parent;

  QString text;
};
