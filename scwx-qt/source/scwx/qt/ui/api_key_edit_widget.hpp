#pragma once

#include <scwx/qt/map/map_provider.hpp>
#include <QNetworkReply>
#include <QLineEdit>

class QNetworkAccessManager;

namespace scwx::qt::ui
{

class QApiKeyEdit : public QLineEdit
{
   Q_OBJECT

public:
   QApiKeyEdit(QWidget* parent = nullptr);

   map::MapProvider getMapProvider() const { return provider_; }

   void setMapProvider(const map::MapProvider provider)
   {
      provider_ = provider;
   }

signals:
   void apiTestSucceeded();
   void apiTestFailed(QNetworkReply::NetworkError error);

private slots:
   void apiTest();
   void apiTestFinished(QNetworkReply* reply);

protected:
   map::MapProvider       provider_ {map::MapProvider::Unknown};
   QNetworkAccessManager* networkAccessManager_ {};
   QAction*               testAction_ {};
};

} // namespace scwx::qt::ui
