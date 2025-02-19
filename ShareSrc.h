/**
 * @file ShareSrc.h
 *
 * @Breife PS模块共享资源
 *
 * @version 1.0
 * @date 2025/1/17
 *
 * @author uxinghai
 * @copyright Copyright (c) 2025
 */

#pragma once

#include <opencv2/core/mat.hpp>
#include <QImage>
#include <QMap>
#include <QPixmap>
#include <QString>
#include <QUrl>

struct FaceLocation {
  double left;
  double top;
  int width;
  int height;
  int rotation;
  FaceLocation() = default;

  FaceLocation(const double left, const double top, const int width,
               const int height, const int rotation = 0)
    : left(left), top(top), width(width), height(height), rotation(rotation) {}
};

struct FaceInfo {
  QString faceGender;          ///< 性别
  int faceAge{};               ///< 年龄
  QString faceShape;           ////< 脸型
  QString faceEmotion;         ///< 情绪
  QString expression;          ////< 笑容
  QString faceType;            ///< type: happy sad
  double faceBeauty{};         ///< 佩戴美丽值
  QString faceGlass;           ///< 佩戴眼镜
  QString faceMask;            ///< 佩戴口罩
  FaceLocation faceLocation{}; ///< 面部在位置
};

// 眼镜映射
const QMap<QString, QString> glassMap = {
  {"none", "未佩戴眼镜"},
  {"sun", "墨镜"},
  {"common", "普通眼镜"},
};

// 笑容映射
const QMap<QString, QString> expressionMap = {
  {"none", "自然"},
  {"smile", "微笑"},
  {"laugh", "大笑"}
};

// 脸型映射
const QMap<QString, QString> faceShapeMap = {
  {"square", "正方形"},
  {"triangle", "三角形"},
  {"oval", "椭圆"},
  {"heart", "心形"},
  {"round", "圆形"}
};

// 情绪映射
const QMap<QString, QString> emotionMap = {
  {"angry", "愤怒"},
  {"disgust", "厌恶"},
  {"fear", "恐惧"},
  {"happy", "高兴"},
  {"sad", "伤心"},
  {"surprise", "惊讶"},
  {"neutral", "自然"},
  {"pouty", "撅嘴"},
  {"grimace", "鬼脸"}
};

// 共享给 threadImageToBase64 线程
inline QImage captureImgCopy; ///< 捕获图像的拷贝

// 共享给 AIDoFaceDetect 线程
inline QString imgBase64Copy; ///< base64编码的拷贝
inline FaceInfo faceInfo;     ///< 共享人脸信息的结构

// URL
// 人脸请求
inline QUrl requestFaceDetectUrl{
  "https://aip.baidubce.com/rest/2.0/face/v3/detect"
};

// 获取token
inline QString getTokenUrl = "https://aip.baidubce.com/oauth/2.0/token";

inline QStringList suffixList{"PNG", "JPG", "JPEG"};

/**
 * @brief 将 QPixmap 转换为 RGB 的 cv::Mat
 * @param pixmap 要转换的 QPixmap 格式图像
 * @return 转换后的 RGB 格式 cv::Mat
 */
inline cv::Mat pixmapToMat(const QPixmap& pixmap)
{
  if (pixmap.isNull()) { return {}; }

  // 将 QPixmap 转换为 QImage
  const QImage img = pixmap.toImage();
  QImage convertedImg = img.convertToFormat(QImage::Format_RGB888);

  // 将 QImage 数据转换为 cv::Mat
  return cv::Mat(convertedImg.height(), convertedImg.width(), CV_8UC3,
                 const_cast<uchar*>(convertedImg.bits()), // const_cast 移除只读属性
                 static_cast<size_t>(convertedImg.bytesPerLine())).clone();
}

/**
 * @brief 将 RGB 格式的 cv::Mat 转换为 QPixmap
 * @param mat 要转换的 cv::Mat 格式图像
 * @return 转换后的 QPixmap 格式图像
 */
inline QPixmap matToPixmap(const cv::Mat& mat)
{
  if (mat.empty()) { return {}; }

  // 检查 Mat 是否为 RGB 格式
  if (mat.type() != CV_8UC3) {
    throw std::invalid_argument("Input cv::Mat must be of type CV_8UC3 (RGB).");
  }

  // 将 cv::Mat 转换为 QImage 并拷贝数据
  const QImage img(mat.data, mat.cols, mat.rows, static_cast<int>(mat.step),
                   QImage::Format_RGB888);

  // QImage 数据拷贝到 QPixmap 中
  return QPixmap::fromImage(img.copy());
}

/**
 * @brief 记录图像的亮度值、对比度值、饱和度值、反相度
 */
struct AdjustValue {
  double brightness = 0;
  double contrast = 10;
  double saturation = 100;
  double contrary = 0;

  bool operator!=(const AdjustValue adjustValue) const
  {
    return brightness != adjustValue.brightness ||
      contrast != adjustValue.contrast ||
      saturation != adjustValue.saturation ||
      contrary != adjustValue.contrary;
  }
};

struct Algorithm {
  int kernelValue = 1;
  QString radioButtonObjName = "radioBlur";

  bool operator!=(const Algorithm& algorithm) const
  {
    return kernelValue != algorithm.kernelValue ||
      radioButtonObjName != algorithm.radioButtonObjName;
  }
};

struct Crop {
  int cropValue = -1;

  bool operator!=(const Crop crop) const { return cropValue != crop.cropValue; }
};

struct Resize {
  int resizeValue = 0;

  bool operator!=(const Resize resize) const
  {
    return resizeValue != resize.resizeValue;
  }
};

struct Rota {
  int rotaValue = 0; ///< 0-360

  bool operator!=(const Rota rota) const { return rotaValue != rota.rotaValue; }
};

struct Filter {
  int filter1 = -1;
  int filter2 = -1;

  bool operator!=(const Filter filter) const
  {
    return filter1 != filter.filter1 || filter2 != filter.filter2;
  }
};

struct MatInfo {
  AdjustValue adjustValue;
  Algorithm algorithm;
  Crop crop;
  Resize resize;
  Rota rota;
  Filter filter;

  bool operator!=(const MatInfo matInfo) const
  {
    return adjustValue != matInfo.adjustValue ||
      algorithm != matInfo.algorithm ||
      crop != matInfo.crop ||
      resize != matInfo.resize ||
      rota != matInfo.rota ||
      filter != matInfo.filter;
  }
};
