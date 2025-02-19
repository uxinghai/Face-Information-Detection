/**
 * @file AIDoFaceDetect.h
 *
 * @Breife None
 *
 * @version 1.0
 * @date 2025/1/17
 *
 * @author uxinghai
 * @copyright Copyright (c) 2025
 */

#pragma once

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QThread>

#include "../Manager/Config/iniManager.h"
#include "../Widgets/photoShop/ShareSrc.h"

class AiDoFaceDetect final : public QThread {
public:
  void run() override
  {
    if (imgBase64Copy.isEmpty()) return;

    auto* faceDetectManager = new QNetworkAccessManager();
    connect(faceDetectManager, &QNetworkAccessManager::finished,
            [this, faceDetectManager](QNetworkReply* reply) {
              reply->deleteLater();
              if (reply->error() == QNetworkReply::NoError) {
                if (auto replyObj = QJsonDocument::fromJson(reply->readAll()).
                    object();
                  replyObj["error_code"].toInt() == 0) {
                  processReplyData(replyObj);
                }
                else {
                  qDebug() << "reply error:" << replyObj["error_code"].toInt()
                    << replyObj["error_msg"].toString();
                }
              }
              else { qCritical() << "reply error:" << reply->error(); }
              faceDetectManager->deleteLater();
              this->quit();
            });

    QNetworkRequest request(requestFaceDetectUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject postJson;
    postJson.insert("image", imgBase64Copy);
    postJson.insert("image_type", "BASE64");
    postJson.insert("face_field",
                    "age,expression,face_shape,gender,"
                    "glasses,emotion,face_type,mask,beauty");
    const QByteArray postData = QJsonDocument(postJson).toJson(
      QJsonDocument::Compact);

    // 发出请求
    faceDetectManager->post(request, postData);

    this->exec();
  }

private:
  static void processReplyData(const QJsonObject& replyObj)
  {
    const auto faceObj = replyObj["result"].toObject()["face_list"].toArray().
      at(0).toObject();

    // 使用结构化绑定简化赋值
    faceInfo.faceGender = faceObj["gender"].toObject()["type"].toString() ==
                          "male"
                            ? "男"
                            : "女";
    faceInfo.faceAge = faceObj["age"].toInt();
    faceInfo.faceBeauty = faceObj["beauty"].toDouble();
    faceInfo.faceMask = faceObj["mask"].toObject()["type"].toInt() == 0
                          ? "未佩戴口罩"
                          : "佩戴口罩";

    // 使用map查找优化
    faceInfo.faceGlass = glassMap.value(
      faceObj["glasses"].toObject()["type"].toString());
    faceInfo.faceEmotion = emotionMap.value(
      faceObj["emotion"].toObject()["type"].toString());
    faceInfo.expression = expressionMap.value(
      faceObj["expression"].toObject()["type"].toString());
    faceInfo.faceShape = faceShapeMap.value(
      faceObj["face_shape"].toObject()["type"].toString());

    // 位置信息处理
    const auto location = faceObj["location"].toObject();
    faceInfo.faceLocation = {
      location["left"].toDouble(),
      location["top"].toDouble(),
      location["width"].toInt(),
      location["height"].toInt(),
      location["rotation"].toInt()
    };
  }
};
