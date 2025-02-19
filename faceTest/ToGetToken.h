/**
 * @file ToGetToken.h
 *
 * @Breife None
 *
 * @version 1.0
 * @date 2025/1/24
 *
 * @author uxinghai
 * @copyright Copyright (c) 2025
 */

#pragma once

#include <QObject>

#include "../../../Network/GetBaiduToken.h"

class ToGetToken final : public QObject {
  Q_OBJECT

public:
  static void getBaiduAIToken()
  {
    const auto settings = iniManager::getIniSetting();

    if (const auto lastDate = settings.value("BaiduAI/LastGetTokenTime").
                                       toDate();
      !lastDate.isValid() || lastDate.addDays(30) < QDate::currentDate()) {
      qDebug() << "request token";
      // 不能使用智能指针，因为析构时线程的工作可能还没完成！
      // 线程的对象析构最好用信号槽
      const auto getBaiduTokenWorker = new GetBaiduToken();
      connect(getBaiduTokenWorker, &GetBaiduToken::baiduTokenReady,
              [](const QString& token) {
                auto setting = iniManager::getIniSetting();
                setting.setValue("BaiduAI/TOKEN", token);
                setting.setValue("BaiduAI/LastGetTokenTime",
                                 QDate::currentDate());
                qDebug() << "get token" << token;

                // 构建人脸检测时用到的 url
                QUrlQuery urlQuery;
                urlQuery.addQueryItem("access_token",
                                      setting.value("BaiduAI/TOKEN").
                                              toString());
                requestFaceDetectUrl.setQuery(urlQuery);
              });
      connect(getBaiduTokenWorker, &GetBaiduToken::finished,
              getBaiduTokenWorker, &QObject::deleteLater); ///< 完成释放内存
      getBaiduTokenWorker->start();
    }
    else {
      // 构建人脸检测时用到的 url
      QUrlQuery urlQuery;
      urlQuery.addQueryItem("access_token",
                            settings.value("BaiduAI/TOKEN").toString());
      requestFaceDetectUrl.setQuery(urlQuery);
    }
  }
};
