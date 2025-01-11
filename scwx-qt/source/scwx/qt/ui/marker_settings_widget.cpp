#include "marker_settings_widget.hpp"
#include "ui_marker_settings_widget.h"

#include <scwx/qt/manager/marker_manager.hpp>
#include <scwx/qt/model/marker_model.hpp>
#include <scwx/qt/types/qt_types.hpp>
#include <scwx/qt/ui/edit_marker_dialog.hpp>
#include <scwx/util/logger.hpp>

#include <QSortFilterProxyModel>

namespace scwx
{
namespace qt
{
namespace ui
{

static const std::string logPrefix_ = "scwx::qt::ui::marker_settings_widget";
static const auto        logger_    = scwx::util::Logger::Create(logPrefix_);

class MarkerSettingsWidgetImpl
{
public:
   explicit MarkerSettingsWidgetImpl(MarkerSettingsWidget* self) :
       self_ {self},
       markerModel_ {new model::MarkerModel(self_)},
       proxyModel_ {new QSortFilterProxyModel(self_)}
   {
      proxyModel_->setSourceModel(markerModel_);
      proxyModel_->setSortRole(Qt::DisplayRole); // TODO types::SortRole
      proxyModel_->setFilterCaseSensitivity(Qt::CaseInsensitive);
      proxyModel_->setFilterKeyColumn(-1);
   }

   void ConnectSignals();

   MarkerSettingsWidget*                   self_;
   model::MarkerModel*                     markerModel_;
   QSortFilterProxyModel*                  proxyModel_;
   std::shared_ptr<manager::MarkerManager> markerManager_ {
      manager::MarkerManager::Instance()};
   std::shared_ptr<ui::EditMarkerDialog> editMarkerDialog_ {nullptr};
};


MarkerSettingsWidget::MarkerSettingsWidget(QWidget* parent) :
    QFrame(parent),
    p {std::make_unique<MarkerSettingsWidgetImpl>(this)},
    ui(new Ui::MarkerSettingsWidget)
{
   ui->setupUi(this);

   ui->removeButton->setEnabled(false);
   ui->markerView->setModel(p->proxyModel_);

   p->editMarkerDialog_ = std::make_shared<ui::EditMarkerDialog>(this);

   p->ConnectSignals();
}

MarkerSettingsWidget::~MarkerSettingsWidget()
{
   delete ui;
}

void MarkerSettingsWidgetImpl::ConnectSignals()
{
   QObject::connect(self_->ui->addButton,
                    &QPushButton::clicked,
                    self_,
                    [this]()
                    {
                       editMarkerDialog_->setup();
                       editMarkerDialog_->show();
                    });
   QObject::connect(
      self_->ui->removeButton,
      &QPushButton::clicked,
      self_,
      [this]()
      {
         auto        selectionModel = self_->ui->markerView->selectionModel();
         QModelIndex selected       = selectionModel
                                   ->selectedRows(static_cast<int>(
                                      model::MarkerModel::Column::Name))
                                   .first();
         std::optional<types::MarkerId> id = markerModel_->getId(selected.row());
         if (!id)
         {
            return;
         }

         markerManager_->remove_marker(*id);
      });
   QObject::connect(
      self_->ui->markerView->selectionModel(),
      &QItemSelectionModel::selectionChanged,
      self_,
      [this](const QItemSelection& selected, const QItemSelection& deselected)
      {
         if (selected.size() == 0 && deselected.size() == 0)
         {
            // Items which stay selected but change their index are not
            // included in selected and deselected. Thus, this signal might
            // be emitted with both selected and deselected empty, if only
            // the indices of selected items change.
            return;
         }

         const bool itemSelected = selected.size() > 0;
         self_->ui->removeButton->setEnabled(itemSelected);
      });
   QObject::connect(self_->ui->markerView,
                    &QAbstractItemView::doubleClicked,
                    self_,
                    [this](const QModelIndex& index)
                    {
                       const int row = index.row();
                       if (row < 0)
                       {
                          return;
                       }

                       std::optional<types::MarkerId> id =
                          markerModel_->getId(row);
                       if (!id)
                       {
                          return;
                       }

                       editMarkerDialog_->setup(*id);
                       editMarkerDialog_->show();
                    });
}

} // namespace ui
} // namespace qt
} // namespace scwx
