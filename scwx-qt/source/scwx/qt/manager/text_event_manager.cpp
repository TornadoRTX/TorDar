#include <scwx/qt/manager/text_event_manager.hpp>
#include <scwx/qt/main/application.hpp>
#include <scwx/qt/settings/general_settings.hpp>
#include <scwx/awips/text_product_file.hpp>
#include <scwx/provider/iem_api_provider.hpp>
#include <scwx/provider/warnings_provider.hpp>
#include <scwx/util/logger.hpp>

#include <shared_mutex>
#include <unordered_map>

#include <boost/asio/post.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/thread_pool.hpp>

namespace scwx
{
namespace qt
{
namespace manager
{

static const std::string logPrefix_ = "scwx::qt::manager::text_event_manager";
static const auto        logger_    = scwx::util::Logger::Create(logPrefix_);

static constexpr std::chrono::hours kInitialLoadHistoryDuration_ =
   std::chrono::days {3};
static constexpr std::chrono::hours kDefaultLoadHistoryDuration_ =
   std::chrono::hours {1};

static const std::array<std::string, 5> kPils_ = {
   "TOR", "SVR", "SVS", "FFW", "FFS"};

class TextEventManager::Impl
{
public:
   explicit Impl(TextEventManager* self) :
       self_ {self},
       refreshTimer_ {threadPool_},
       refreshMutex_ {},
       textEventMap_ {},
       textEventMutex_ {}
   {
      auto& generalSettings = settings::GeneralSettings::Instance();

      warningsProvider_ = std::make_shared<provider::WarningsProvider>(
         generalSettings.warnings_provider().GetValue());

      warningsProviderChangedCallbackUuid_ =
         generalSettings.warnings_provider().RegisterValueChangedCallback(
            [this](const std::string& value)
            {
               loadHistoryDuration_ = kInitialLoadHistoryDuration_;
               warningsProvider_ =
                  std::make_shared<provider::WarningsProvider>(value);
            });

      boost::asio::post(threadPool_,
                        [this]()
                        {
                           try
                           {
                              main::Application::WaitForInitialization();
                              logger_->debug("Start Refresh");
                              Refresh();
                           }
                           catch (const std::exception& ex)
                           {
                              logger_->error(ex.what());
                           }
                        });
   }

   ~Impl()
   {
      settings::GeneralSettings::Instance()
         .warnings_provider()
         .UnregisterValueChangedCallback(warningsProviderChangedCallbackUuid_);

      std::unique_lock lock(refreshMutex_);
      refreshTimer_.cancel();
      lock.unlock();

      threadPool_.join();
   }

   void
   HandleMessage(const std::shared_ptr<awips::TextProductMessage>& message);
   void LoadArchive(std::chrono::sys_days date, const std::string& pil);
   void LoadArchives(std::chrono::sys_days date);
   void RefreshAsync();
   void Refresh();

   // Thread pool sized for:
   // - Live Refresh (1x)
   // - Archive Loading (15x)
   //   - 3 day window (3x)
   //   - TOR, SVR, SVS, FFW, FFS (5x)
   boost::asio::thread_pool threadPool_ {16u};

   TextEventManager* self_;

   boost::asio::steady_timer refreshTimer_;
   std::mutex                refreshMutex_;

   std::unordered_map<types::TextEventKey,
                      std::vector<std::shared_ptr<awips::TextProductMessage>>,
                      types::TextEventHash<types::TextEventKey>>
                     textEventMap_;
   std::shared_mutex textEventMutex_;

   std::unique_ptr<provider::IemApiProvider> iemApiProvider_ {
      std::make_unique<provider::IemApiProvider>()};
   std::shared_ptr<provider::WarningsProvider> warningsProvider_ {nullptr};
   std::chrono::hours loadHistoryDuration_ {kInitialLoadHistoryDuration_};
   std::chrono::sys_time<std::chrono::hours> prevLoadTime_ {};

   boost::uuids::uuid warningsProviderChangedCallbackUuid_ {};
};

TextEventManager::TextEventManager() : p(std::make_unique<Impl>(this)) {}
TextEventManager::~TextEventManager() = default;

size_t TextEventManager::message_count(const types::TextEventKey& key) const
{
   size_t messageCount = 0u;

   std::shared_lock lock(p->textEventMutex_);

   auto it = p->textEventMap_.find(key);
   if (it != p->textEventMap_.cend())
   {
      messageCount = it->second.size();
   }

   return messageCount;
}

std::vector<std::shared_ptr<awips::TextProductMessage>>
TextEventManager::message_list(const types::TextEventKey& key) const
{
   std::vector<std::shared_ptr<awips::TextProductMessage>> messageList {};

   std::shared_lock lock(p->textEventMutex_);

   auto it = p->textEventMap_.find(key);
   if (it != p->textEventMap_.cend())
   {
      messageList.assign(it->second.begin(), it->second.end());
   }

   return messageList;
}

void TextEventManager::LoadFile(const std::string& filename)
{
   logger_->debug("LoadFile: {}", filename);

   boost::asio::post(p->threadPool_,
                     [=, this]()
                     {
                        try
                        {
                           awips::TextProductFile file;

                           // Load file
                           bool fileLoaded = file.LoadFile(filename);
                           if (!fileLoaded)
                           {
                              return;
                           }

                           // Process messages
                           auto messages = file.messages();
                           for (auto& message : messages)
                           {
                              p->HandleMessage(message);
                           }
                        }
                        catch (const std::exception& ex)
                        {
                           logger_->error(ex.what());
                        }
                     });
}

void TextEventManager::SelectTime(
   std::chrono::system_clock::time_point dateTime)
{
   const auto today     = std::chrono::floor<std::chrono::days>(dateTime);
   const auto yesterday = today - std::chrono::days {1};
   const auto tomorrow  = today + std::chrono::days {1};
   const auto dates     = {yesterday, today, tomorrow};

   for (auto& date : dates)
   {
      p->LoadArchives(date);
   }
}

void TextEventManager::Impl::HandleMessage(
   const std::shared_ptr<awips::TextProductMessage>& message)
{
   auto segments = message->segments();

   // If there are no segments, skip this message
   if (segments.empty())
   {
      return;
   }

   for (auto& segment : segments)
   {
      // If a segment has no header, or if there is no VTEC string, skip this
      // message. A segmented message corresponding to a text event should have
      // this information.
      if (!segment->header_.has_value() ||
          segment->header_->vtecString_.empty())
      {
         return;
      }
   }

   std::unique_lock lock(textEventMutex_);

   // Find a matching event in the event map
   auto&               vtecString = segments[0]->header_->vtecString_;
   types::TextEventKey key {vtecString[0].pVtec_};
   size_t              messageIndex = 0;
   auto                it           = textEventMap_.find(key);
   bool                updated      = false;

   if (it == textEventMap_.cend())
   {
      // If there was no matching event, add the message to a new event
      textEventMap_.emplace(key, std::vector {message});
      messageIndex = 0;
      updated      = true;
   }
   else if (std::find_if(it->second.cbegin(),
                         it->second.cend(),
                         [=](auto& storedMessage)
                         {
                            return *message->wmo_header().get() ==
                                   *storedMessage->wmo_header().get();
                         }) == it->second.cend())
   {
      // If there was a matching event, and this message has not been stored
      // (WMO header equivalence check), add the updated message to the existing
      // event

      // Determine the chronological sequence of the message. Note, if there
      // were no time hints given to the WMO header, this will place the message
      // at the end of the vector.
      auto insertionPoint = std::upper_bound(
         it->second.begin(),
         it->second.end(),
         message,
         [](const std::shared_ptr<awips::TextProductMessage>& a,
            const std::shared_ptr<awips::TextProductMessage>& b) {
            return a->wmo_header()->GetDateTime() <
                   b->wmo_header()->GetDateTime();
         });

      // Insert the message in chronological order
      messageIndex = std::distance(it->second.begin(), insertionPoint);
      it->second.insert(insertionPoint, message);
      updated = true;
   };

   lock.unlock();

   if (updated)
   {
      Q_EMIT self_->AlertUpdated(key, messageIndex, message->uuid());
   }
}

void TextEventManager::Impl::LoadArchive(std::chrono::sys_days date,
                                         const std::string&    pil)
{
   const auto& productIds = iemApiProvider_->ListTextProducts(date, {}, pil);
   const auto& products   = iemApiProvider_->LoadTextProducts(productIds);

   for (auto& product : products)
   {
      const auto& messages = product->messages();

      for (auto& message : messages)
      {
         HandleMessage(message);
      }
   }
}

void TextEventManager::Impl::LoadArchives(std::chrono::sys_days date)
{
   for (auto& pil : kPils_)
   {
      boost::asio::post(threadPool_,
                        [this, date, &pil]() { LoadArchive(date, pil); });
   }
}

void TextEventManager::Impl::RefreshAsync()
{
   boost::asio::post(threadPool_,
                     [this]()
                     {
                        try
                        {
                           Refresh();
                        }
                        catch (const std::exception& ex)
                        {
                           logger_->error(ex.what());
                        }
                     });
}

void TextEventManager::Impl::Refresh()
{
   logger_->trace("Refresh");

   // Take a unique lock before refreshing
   std::unique_lock lock(refreshMutex_);

   // Take a copy of the current warnings provider to protect against change
   std::shared_ptr<provider::WarningsProvider> warningsProvider =
      warningsProvider_;

   // Load updated files from the warnings provider
   // Start time should default to:
   // - 3 days of history for the first load
   // - 1 hour of history for subsequent loads
   // If the time jumps, we should attempt to load from no later than the
   // previous load time
   auto loadTime =
      std::chrono::floor<std::chrono::hours>(std::chrono::system_clock::now());
   auto startTime = loadTime - loadHistoryDuration_;

   if (prevLoadTime_ != std::chrono::sys_time<std::chrono::hours> {})
   {
      startTime = std::min(startTime, prevLoadTime_);
   }

   auto updatedFiles = warningsProvider->LoadUpdatedFiles(startTime);

   // Store the load time and reset the load history duration
   prevLoadTime_        = loadTime;
   loadHistoryDuration_ = kDefaultLoadHistoryDuration_;

   // Handle messages
   for (auto& file : updatedFiles)
   {
      for (auto& message : file->messages())
      {
         HandleMessage(message);
      }
   }

   // Schedule another update in 15 seconds
   using namespace std::chrono;
   refreshTimer_.expires_after(15s);
   refreshTimer_.async_wait(
      [this](const boost::system::error_code& e)
      {
         if (e == boost::asio::error::operation_aborted)
         {
            logger_->debug("Refresh timer cancelled");
         }
         else if (e != boost::system::errc::success)
         {
            logger_->warn("Refresh timer error: {}", e.message());
         }
         else
         {
            RefreshAsync();
         }
      });
}

std::shared_ptr<TextEventManager> TextEventManager::Instance()
{
   static std::weak_ptr<TextEventManager> textEventManagerReference_ {};
   static std::mutex                      instanceMutex_ {};

   std::unique_lock lock(instanceMutex_);

   std::shared_ptr<TextEventManager> textEventManager =
      textEventManagerReference_.lock();

   if (textEventManager == nullptr)
   {
      textEventManager           = std::make_shared<TextEventManager>();
      textEventManagerReference_ = textEventManager;
   }

   return textEventManager;
}

} // namespace manager
} // namespace qt
} // namespace scwx
