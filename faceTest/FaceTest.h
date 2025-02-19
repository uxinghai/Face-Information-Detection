/**
* @file FaceTest.h
 *
 * @brief 人脸信息检测模块
 *
 * @version 1.0
 * @date 2025/01/14
 *
 * @author uxinghai
 * @copyright Copyright (c) 2025
 */

#pragma once

#include <QMainWindow>
#include <QNetworkAccessManager>

class AiDoFaceDetect;
QT_BEGIN_NAMESPACE

namespace Ui
{
  class FaceTest;
}

class QCamera;
class QMediaCaptureSession;
class QImageCapture;
class QTimer;
class QLabel;
class QComboBox;
class QDoubleSpinBox;
class QPainter;
class QNetworkAccessManager;
class ThreadImageToBase64;
class QUrl;
QT_END_NAMESPACE

class FaceTest final : public QMainWindow {
  Q_OBJECT

public:
  FaceTest();
  ~FaceTest() override;

private slots:
  void doOpenCamera();
  void doCloseCamera() const;
  void doFaceDetect() const;

private:
  void init();
  void setupConnections();
  void setActionEnable(bool enable) const;
  void showPicture(const QImage& preview) const;

  Ui::FaceTest* ui;
  QMediaCaptureSession* captureSession;
  QCamera* camera;
  QImageCapture* imageCapture;
  std::unique_ptr<QTimer> captureTimer;
  std::unique_ptr<QTimer> getBase64Timer;
  std::unique_ptr<QTimer> doClickDetectTimer;

  QLabel* lbInfo;
  QLabel* lbParInfo;
  QComboBox* cameraBox{};
  QDoubleSpinBox* doubleSpinBox{};

  std::unique_ptr<ThreadImageToBase64> imgToBase64;
  std::unique_ptr<AiDoFaceDetect> faceDetect;
  bool slipFlag{true};
};
