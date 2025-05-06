#pragma once

#include <memory>

#include <QSortFilterProxyModel>

namespace scwx::qt::model
{

class AlertProxyModelImpl;

class AlertProxyModel : public QSortFilterProxyModel
{
private:
   Q_DISABLE_COPY_MOVE(AlertProxyModel)

public:
   explicit AlertProxyModel(QObject* parent = nullptr);
   ~AlertProxyModel();

   void SetAlertActiveFilter(bool enabled);

   [[nodiscard]] bool
   filterAcceptsRow(int                sourceRow,
                    const QModelIndex& sourceParent) const override;

private:
   class Impl;
   std::unique_ptr<Impl> p;
};

} // namespace scwx::qt::model
