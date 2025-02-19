/**
* @file ThreadImageToBase64.h
 *
 * @Breife 线程版本
 *
 * @version 1.0
 * @date 2025/1/15
 *
 * @author uxinghai
 * @copyright Copyright (c) 2025
 */

#pragma once

#include <QBuffer>
#include <QImage>
#include <QThread>

class ThreadImageToBase64 final : public QThread {
  Q_OBJECT

public:
  // 优化图像转base64编码速度
  void run() override
  {
    if (captureImgCopy.isNull()) return;

    // 估算所需大小预分配内存
    QByteArray byteArray;
    byteArray.reserve(captureImgCopy.width() * captureImgCopy.height() * 4);

    // 使用内存缓冲
    QBuffer buf(&byteArray);
    buf.open(QIODevice::WriteOnly);

    // 优化图片格式和质量
    captureImgCopy.save(&buf, "jpg", 85); ///< 使用jpg格式,质量85%

    // 使用更快的Base64编码选项
    const auto base64 = byteArray.toBase64(
      QByteArray::Base64Encoding | QByteArray::OmitTrailingEquals);

    emit base64Ready(base64);
  }

signals:
  void base64Ready(const QString& base64);
};
