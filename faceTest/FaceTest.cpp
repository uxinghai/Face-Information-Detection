#include "FaceTest.h"

#include <QCamera>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QImageCapture>
#include <QMediaCaptureSession>
#include <QMediaDevices>
#include <QPainter>
#include <QThreadPool>
#include <QTimer>

#include "../../../../UI/ui_FaceTest.h"
#include "../../../Network/AIDoFaceDetect.h"
#include "../../../Utils/threadWorkers/ThreadImageToBase64.h"
#include "../../../Utils/Tools/MyInformationBox.h"
#include "../ShareSrc.h"

FaceTest::FaceTest()
  : ui(new Ui::FaceTest),
    captureSession(new QMediaCaptureSession(this)),
    camera(new QCamera()),
    imageCapture(new QImageCapture()),
    captureTimer(std::make_unique<QTimer>()),
    getBase64Timer(std::make_unique<QTimer>()),
    doClickDetectTimer(std::make_unique<QTimer>()),
    lbInfo(new QLabel(this)),
    lbParInfo(new QLabel(this)),
    imgToBase64(std::make_unique<ThreadImageToBase64>()),
    faceDetect(std::make_unique<AiDoFaceDetect>())
{
  ui->setupUi(this);
  init();
  setupConnections();
}

void FaceTest::init()
{
  captureSession->setCamera(camera);
  captureSession->setImageCapture(imageCapture);
  lbInfo->setText(tr("打开摄像头获取检测信息."));
  ui->statusBar->addWidget(lbInfo);
  lbParInfo->setText(tr("AI检测，仅供参考."));
  ui->statusBar->addPermanentWidget(lbParInfo);

  // toolBar嵌入新工具
  auto* delayIcon = new QLabel();
  auto* delayText = new QLabel(tr("延迟(秒)："));
  doubleSpinBox = new QDoubleSpinBox();
  cameraBox = new QComboBox();

  delayIcon->setFixedSize(24, 24);
  delayIcon->setScaledContents(true);
  delayIcon->setPixmap(QPixmap(":/ps/res/ps/cameraDelay.png"));
  auto font = delayText->font();
  font.setPointSize(11);
  delayText->setFont(font);
  doubleSpinBox->setRange(0.01, 10); ///< s/1000->ms
  doubleSpinBox->setValue(0.05);
  doubleSpinBox->setSingleStep(0.01);
  for (const auto& cameraName : QMediaDevices::videoInputs()) {
    cameraBox->addItem(cameraName.description());
  }
  cameraBox->setCurrentIndex(0);

  ui->toolBar->addWidget(delayIcon);
  ui->toolBar->addWidget(delayText);
  ui->toolBar->addWidget(doubleSpinBox);
  ui->toolBar->addSeparator();
  ui->toolBar->addWidget(cameraBox);
}

void FaceTest::setupConnections()
{
  connect(captureTimer.get(), &QTimer::timeout,
          [this] { imageCapture->capture(); });
  connect(imageCapture, &QImageCapture::imageCaptured,
          [this](const int id, const QImage& preview) {
            Q_UNUSED(id);
            showPicture(preview);

            lbInfo->setText(tr("已开启摄像头;") +
              (ui->actionSlip->isChecked() ? tr("启用镜像;") : tr("未启用镜像;")) +
              tr("相机延迟为：") + QString::number(doubleSpinBox->value()));
            captureTimer->setInterval(
              static_cast<int>(doubleSpinBox->value() * 1000));
          });
  connect(getBase64Timer.get(), &QTimer::timeout, [this] {
    if (imgToBase64->isRunning()) return;
    imgToBase64->start();
  });
  connect(imgToBase64.get(), &ThreadImageToBase64::base64Ready,
          [this](const QString& base64) { imgBase64Copy = base64; });

  // 工具栏操作
  connect(ui->actionOpenCamera, &QAction::triggered, this,
          &FaceTest::doOpenCamera);
  connect(ui->actionCloseCamera, &QAction::triggered, this,
          &FaceTest::doCloseCamera);
  connect(ui->actionSlip, &QAction::triggered, [this](const bool checked) {
    slipFlag = checked;
  });
  connect(cameraBox, &QComboBox::currentTextChanged,
          [this](const QString& text) {
            camera->setCameraDevice(QMediaDevices::videoInputs().at(
              cameraBox->findText(text)));
          });

  connect(doClickDetectTimer.get(), &QTimer::timeout, this,
          &FaceTest::doFaceDetect);
}

void FaceTest::doOpenCamera()
{
  camera->start();
  if (!camera->isActive()) {
    auto* infoBox = new MyInformationBox(this, tr("相机打开失败\n请重试"));
    infoBox->show();
    return;
  }
  // 启动两个计时器
  captureTimer->start(0);
  getBase64Timer->start(0);
  doClickDetectTimer->start(0);
  ui->actionOpenCamera->setIcon(QIcon(":/ps/res/ps/camera_O.png"));

  setActionEnable(true);
  qDebug() << "Camera is open.";
}

void FaceTest::doCloseCamera() const
{
  setActionEnable(false);
  ui->actionOpenCamera->setIcon(QIcon(":/ps/res/ps/camera_C.png"));
  camera->stop();
  captureTimer->stop();
  getBase64Timer->stop();
  doClickDetectTimer->stop();
  ui->lbCamera->clear();
  // 如果线程还在运行强制停止
  if (imgToBase64->isRunning()) { imgToBase64->terminate(); }
  if (faceDetect->isRunning()) { faceDetect->terminate(); }
  lbInfo->setText(tr("打开摄像头获取检测信息."));
  qDebug() << "Camera is close";
}

void FaceTest::doFaceDetect() const
{
  if (faceDetect->isRunning()) { return; }
  faceDetect->start();
}

void FaceTest::setActionEnable(const bool enable) const
{
  ui->actionOpenCamera->setEnabled(!enable);
  ui->actionCloseCamera->setEnabled(enable);
  ui->actionSlip->setEnabled(enable);
}

void FaceTest::showPicture(const QImage& preview) const
{
  captureImgCopy = slipFlag ? preview.mirrored(true, false) : preview;

  QPainter painter(&captureImgCopy);

  // 设置人脸框样式
  painter.setPen(QPen(Qt::red, 2));
  const auto& loc = faceInfo.faceLocation;
  painter.drawRect(loc.left, loc.top, loc.width, loc.height);

  // 设置文字样式
  QFont font = painter.font();
  font.setPointSize(16);
  painter.setFont(font);

  const double textX = loc.left + loc.width + 5;
  const double baseY = loc.top;

  // 使用结构化方式绘制文本信息
  const QVector<QString> textItems = {
    "性别:" + faceInfo.faceGender,
    "年龄:" + QString::number(faceInfo.faceAge),
    "脸型:" + faceInfo.faceShape,
    "表情:" + faceInfo.expression,
    "口罩:" + faceInfo.faceMask,
    "眼镜:" + faceInfo.faceGlass,
    "颜值:" + QString::number(faceInfo.faceBeauty)
  };

  for (size_t i = 0; i < textItems.size(); ++i) {
    constexpr int lineHeight = 25;
    painter.drawText(QPointF(textX, baseY + (i + 1) * lineHeight),
                     textItems[i]);
  }

  ui->lbCamera->setPixmap(QPixmap::fromImage(captureImgCopy));
}

FaceTest::~FaceTest()
{
  doCloseCamera();
  delete ui;
}
