#include <scwx/qt/map/lightning_layer.hpp>

#include <scwx/gr/placefile.hpp>
#include <scwx/qt/manager/lightning_manager.hpp>
#include <scwx/qt/manager/timeline_manager.hpp>
#include <scwx/qt/gl/gl.hpp>
#include <scwx/util/logger.hpp>

#include <boost/asio/thread_pool.hpp>
#include <boost/asio/post.hpp>

#include <chrono>
#include <mutex>
#include <vector>

namespace scwx::qt::map
{

static const std::string logPrefix_ = "scwx::qt::map::lightning_layer";
static const auto        logger_    = scwx::util::Logger::Create(logPrefix_);

class LightningLayer::Impl
{
public:
   explicit Impl(LightningLayer*                       self,
                 const std::shared_ptr<gl::GlContext>& glContext) :
       self_ {self},
       placefileIcons_ {std::make_shared<gl::draw::PlacefileIcons>(glContext)}
   {
      ConnectSignals();
   }

   ~Impl() = default;

   Impl(const Impl&)                = delete;
   Impl(Impl&&) noexcept            = delete;
   Impl& operator=(const Impl&)     = delete;
   Impl& operator=(Impl&&) noexcept = delete;

   void ConnectSignals();
   void ReloadDataSync();

   boost::asio::thread_pool threadPool_ {1};
   std::mutex               dataMutex_ {};

   LightningLayer*                           self_;
   std::shared_ptr<gl::draw::PlacefileIcons> placefileIcons_ {};
   std::chrono::system_clock::time_point     selectedTime_ {};
};

void LightningLayer::Impl::ConnectSignals()
{
   auto lightningManager = manager::LightningManager::Instance();
   auto timelineManager  = manager::TimelineManager::Instance();

   QObject::connect(lightningManager.get(),
                    &manager::LightningManager::LightningUpdated,
                    self_,
                    [this]() { self_->ReloadData(); });

   QObject::connect(timelineManager.get(),
                    &manager::TimelineManager::SelectedTimeUpdated,
                    self_,
                    [this](std::chrono::system_clock::time_point dateTime)
                    { selectedTime_ = dateTime; });
}

void LightningLayer::Impl::ReloadDataSync()
{
   logger_->debug("ReloadData: lightning");

   std::unique_lock lock {dataMutex_};

   auto lightningManager = manager::LightningManager::Instance();
   auto placefile        = lightningManager->placefile();

   if (placefile != nullptr)
   {
      placefileIcons_->SetIconFiles(placefile->icon_files(), placefile->name());
   }
   else
   {
      placefileIcons_->SetIconFiles(
         std::vector<std::shared_ptr<const gr::Placefile::IconFile>> {},
         std::string {});
   }

   placefileIcons_->StartIcons();
   if (placefile != nullptr)
   {
      for (const auto& drawItem : placefile->GetDrawItems())
      {
         if (drawItem->itemType_ == gr::Placefile::ItemType::Icon)
         {
            placefileIcons_->AddIcon(
               std::static_pointer_cast<gr::Placefile::IconDrawItem>(drawItem));
         }
      }
   }
   placefileIcons_->FinishIcons();

   Q_EMIT self_->DataReloaded();
}

LightningLayer::LightningLayer(
   const std::shared_ptr<gl::GlContext>& glContext) :
    p(std::make_unique<Impl>(this, glContext))
{
   AddDrawItem(p->placefileIcons_);
   ReloadData();
}

LightningLayer::~LightningLayer() = default;

void LightningLayer::Initialize(const std::shared_ptr<MapContext>& mapContext)
{
   DrawLayer::Initialize(mapContext);
   p->selectedTime_ = manager::TimelineManager::Instance()->GetSelectedTime();
}

void LightningLayer::Render(
   const std::shared_ptr<MapContext>&            mapContext,
   const QMapLibre::CustomLayerRenderParameters& params)
{
   if (p->placefileIcons_ != nullptr)
   {
      p->placefileIcons_->set_selected_time(p->selectedTime_);
      p->placefileIcons_->set_thresholded(false);
      p->placefileIcons_->set_time_fade_enabled(true);
   }

   DrawLayer::Render(mapContext, params);
   SCWX_GL_CHECK_ERROR();
}

void LightningLayer::Deinitialize()
{
   DrawLayer::Deinitialize();
}

void LightningLayer::ReloadData()
{
   boost::asio::post(p->threadPool_,
                     [this]()
                     {
                        try
                        {
                           p->ReloadDataSync();
                        }
                        catch (const std::exception& ex)
                        {
                           logger_->error(ex.what());
                        }
                     });
}

} // namespace scwx::qt::map
