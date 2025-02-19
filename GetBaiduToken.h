/**
* @file GetBaiduToken.h
 *
 * @Breife None
 *
 * @version 1.0
 * @date 2025/1/14
 *
 * @author uxinghai
 * @copyright Copyright (c) 2025
 */
#pragma once

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QThread>
#include <QUrlQuery>

#include "../Manager/Config/iniManager.h"
#include "../Widgets/photoShop/ShareSrc.h"

class GetBaiduToken final : public QThread {
  Q_OBJECT

public:
  explicit GetBaiduToken(QObject* parent = nullptr) : QThread(parent)
  {
    // 此处不能放 faceDetectManager 的实例化，构造函数由主线程调用
    // 而 run 会自动的分配另一个线程去调用 即在另一个线程中使用主线程的manager不对！！！
  }

  // 通过 exec() 保证事件循环的正常运行
  void run() override
  {
    manager = std::make_unique<QNetworkAccessManager>();

    connect(manager.get(), &QNetworkAccessManager::finished,
            [this](QNetworkReply* reply) {
              if (reply->error() == QNetworkReply::NoError) {
                if (const QString replyText = reply->readAll();
                  !replyText.contains("error")) {
                  // 得到回复后释放内存并发出信号
                  reply->deleteLater();
                  QJsonObject obj = QJsonDocument::fromJson(replyText.toUtf8())
                    .object();
                  const auto token = obj["access_token"].toString();
                  emit baiduTokenReady(token);

                  // 需要在事件响应之后手动加上
                  this->quit();
                }
              }
            });

    const auto baiduAi = iniManager::getIniSetting();
    const auto API_KEY = baiduAi.value("BaiduAI/API_KEY").toString();
    const auto SECRET_KEY = baiduAi.value("BaiduAI/SECRET_KEY").toString();
    const QString url = getTokenUrl +
      "?grant_type=client_credentials&client_id=" +
      API_KEY + "&client_secret=" + SECRET_KEY;

    manager->get(QNetworkRequest(QUrl(url))); ///< 发出get请求

    // thread 类带的exec
    this->exec(); ///< 事件循环等待 finished 的响应，否则线程直接退出
  }

signals:
  void baiduTokenReady(const QString& token);

private:
  std::unique_ptr<QNetworkAccessManager> manager;
};
