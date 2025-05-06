#pragma once

#include <scwx/qt/types/text_event_key.hpp>
#include <scwx/common/geographic.hpp>

#include <memory>
#include <unordered_set>

#include <boost/uuid/uuid.hpp>
#include <QAbstractTableModel>

namespace scwx
{
namespace qt
{
namespace model
{

class AlertModelImpl;

class AlertModel : public QAbstractTableModel
{
public:
   enum class Column : int
   {
      Etn            = 0,
      OfficeId       = 1,
      Phenomenon     = 2,
      Significance   = 3,
      ThreatCategory = 4,
      Tornado        = 5,
      State          = 6,
      Counties       = 7,
      StartTime      = 8,
      EndTime        = 9,
      Distance       = 10
   };

   explicit AlertModel(QObject* parent = nullptr);
   ~AlertModel();

   types::TextEventKey key(const QModelIndex& index) const;
   common::Coordinate  centroid(const types::TextEventKey& key) const;

   int rowCount(const QModelIndex& parent = QModelIndex()) const override;
   int columnCount(const QModelIndex& parent = QModelIndex()) const override;

   QVariant data(const QModelIndex& index,
                 int                role = Qt::DisplayRole) const override;
   QVariant headerData(int             section,
                       Qt::Orientation orientation,
                       int             role = Qt::DisplayRole) const override;

public slots:
   void HandleAlert(const types::TextEventKey& alertKey,
                    std::size_t                messageIndex,
                    boost::uuids::uuid         uuid);
   void HandleAlertsRemoved(
      const std::unordered_set<types::TextEventKey,
                               types::TextEventHash<types::TextEventKey>>&
         alertKeys);
   void HandleMapUpdate(double latitude, double longitude);

private:
   std::unique_ptr<AlertModelImpl> p;

   friend class AlertModelImpl;
};

} // namespace model
} // namespace qt
} // namespace scwx
