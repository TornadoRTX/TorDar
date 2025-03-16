#include "custom_layer_dialog.hpp"
#include "ui_custom_layer_dialog.h"

#include <re2/re2.h>
#include <scwx/qt/settings/general_settings.hpp>
#include <scwx/util/logger.hpp>
#include <scwx/qt/map/map_provider.hpp>
#include <utility>

namespace scwx::qt::ui
{

static const std::string logPrefix_ = "scwx::qt::ui::custom_layer_dialog";
static const auto        logger_    = scwx::util::Logger::Create(logPrefix_);

class CustomLayerDialogImpl
{
public:
   explicit CustomLayerDialogImpl(CustomLayerDialog*  self,
                                  QMapLibre::Settings settings) :
       self_(self), settings_(std::move(settings))
   {
   }

   ~CustomLayerDialogImpl()                                       = default;
   CustomLayerDialogImpl(const CustomLayerDialogImpl&)            = delete;
   CustomLayerDialogImpl(CustomLayerDialogImpl&&)                 = delete;
   CustomLayerDialogImpl& operator=(const CustomLayerDialogImpl&) = delete;
   CustomLayerDialogImpl& operator=(CustomLayerDialogImpl&&)      = delete;

   void handle_mapChanged(QMapLibre::Map::MapChange change);

   CustomLayerDialog* self_;

   QMapLibre::Settings             settings_;
   std::shared_ptr<QMapLibre::Map> map_;
};

// TODO Duplicated form map_widget, Should probably be moved.
static bool match_layer(const std::string& pattern, const std::string& layer)
{
   // Perform case-insensitive matching
   const RE2 re {"(?i)" + pattern};
   if (re.ok())
   {
      return RE2::FullMatch(layer, re);
   }
   else
   {
      // Fall back to basic comparison if RE
      // doesn't compile
      return layer == pattern;
   }
}

void CustomLayerDialogImpl::handle_mapChanged(QMapLibre::Map::MapChange change)
{
   if (change == QMapLibre::Map::MapChange::MapChangeDidFinishLoadingStyle)
   {
      auto& generalSettings = settings::GeneralSettings::Instance();
      const std::string& customStyleDrawLayer =
         generalSettings.custom_style_draw_layer().GetValue();

      const QStringList layerIds = map_->layerIds();
      self_->ui->layerListWidget->clear();
      self_->ui->layerListWidget->addItems(layerIds);

      for (int i = 0; i < self_->ui->layerListWidget->count(); i++)
      {
         auto* item = self_->ui->layerListWidget->item(i);

         if (match_layer(customStyleDrawLayer, item->text().toStdString()))
         {
            self_->ui->layerListWidget->setCurrentItem(item);
         }
      }
   }
}

CustomLayerDialog::CustomLayerDialog(const QMapLibre::Settings& settings,
                                     QWidget*                   parent) :
    QDialog(parent),
    p {std::make_unique<CustomLayerDialogImpl>(this, settings)},
    ui(new Ui::CustomLayerDialog)
{
   ui->setupUi(this);

   auto&       generalSettings = settings::GeneralSettings::Instance();
   const auto& customStyleUrl  = generalSettings.custom_style_url().GetValue();
   const auto  mapProvider =
      map::GetMapProvider(generalSettings.map_provider().GetValue());

   // TODO render the map with a layer to show what they are selecting
   p->map_ = std::make_shared<QMapLibre::Map>(
      nullptr, p->settings_, QSize(1, 1), devicePixelRatioF());

   QString qUrl = QString::fromStdString(customStyleUrl);

   if (mapProvider == map::MapProvider::MapTiler)
   {
      qUrl.append("?key=");
      qUrl.append(map::GetMapProviderApiKey(mapProvider));
   }

   p->map_->setStyleUrl(qUrl);

   QObject::connect(p->map_.get(),
                    &QMapLibre::Map::mapChanged,
                    this,
                    [this](QMapLibre::Map::MapChange change)
                    { p->handle_mapChanged(change); });
}

CustomLayerDialog::~CustomLayerDialog()
{
   delete ui;
}

std::string CustomLayerDialog::selected_layer()
{
   return ui->layerListWidget->currentItem()->text().toStdString();
}

} // namespace scwx::qt::ui
