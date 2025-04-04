#include <scwx/provider/aws_level2_chunks_data_provider.hpp>
#include <scwx/util/environment.hpp>
#include <scwx/util/map.hpp>
#include <scwx/util/logger.hpp>
#include <scwx/util/time.hpp>
#include <scwx/wsr88d/ar2v_file.hpp>

#include <shared_mutex>
#include <utility>

#include <aws/core/auth/AWSCredentials.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/ListObjectsV2Request.h>
#include <boost/timer/timer.hpp>
#include <fmt/chrono.h>
#include <fmt/format.h>

#if (__cpp_lib_chrono < 201907L)
#   include <date/date.h>
#endif

namespace scwx::provider
{
static const std::string logPrefix_ =
   "scwx::provider::aws_level2_chunks_data_provider";
static const auto logger_ = util::Logger::Create(logPrefix_);

static const std::string kDefaultBucketName_ = "unidata-nexrad-level2-chunks";
static const std::string kDefaultRegion_     = "us-east-1";

class AwsLevel2ChunksDataProvider::Impl
{
public:
   struct ScanRecord
   {
      explicit ScanRecord(std::string prefix, bool valid = true) :
          valid_ {valid},
          prefix_ {std::move(prefix)},
          nexradFile_ {},
          lastModified_ {},
          lastKey_ {""}
      {
      }
      ~ScanRecord()                            = default;
      ScanRecord(const ScanRecord&)            = default;
      ScanRecord(ScanRecord&&)                 = default;
      ScanRecord& operator=(const ScanRecord&) = default;
      ScanRecord& operator=(ScanRecord&&)      = default;

      bool                                  valid_;
      std::string                           prefix_;
      std::shared_ptr<wsr88d::Ar2vFile>     nexradFile_;
      std::chrono::system_clock::time_point time_;
      std::chrono::system_clock::time_point lastModified_;
      std::chrono::system_clock::time_point secondLastModified_;
      std::string                           lastKey_;
      int                                   nextFile_ {1};
      bool                                  hasAllFiles_ {false};
   };

   explicit Impl(AwsLevel2ChunksDataProvider* self,
                 std::string                  radarSite,
                 std::string                  bucketName,
                 std::string                  region) :
       radarSite_ {std::move(radarSite)},
       bucketName_ {std::move(bucketName)},
       region_ {std::move(region)},
       client_ {nullptr},
       scanTimes_ {},
       lastScan_ {"", false},
       currentScan_ {"", false},
       scansMutex_ {},
       lastTimeListed_ {},
       updatePeriod_ {7},
       self_ {self}
   {
      // Disable HTTP request for region
      util::SetEnvironment("AWS_EC2_METADATA_DISABLED", "true");

      // Use anonymous credentials
      Aws::Auth::AWSCredentials credentials {};

      Aws::Client::ClientConfiguration config;
      config.region           = region_;
      config.connectTimeoutMs = 10000;

      client_ = std::make_shared<Aws::S3::S3Client>(
         credentials,
         Aws::MakeShared<Aws::S3::S3EndpointProvider>(
            Aws::S3::S3Client::GetAllocationTag()),
         config);
   }
   ~Impl()                      = default;
   Impl(const Impl&)            = delete;
   Impl(Impl&&)                 = delete;
   Impl& operator=(const Impl&) = delete;
   Impl& operator=(Impl&&)      = delete;

   std::chrono::system_clock::time_point GetScanTime(const std::string& prefix);
   int         GetScanNumber(const std::string& prefix);
   std::string GetScanKey(const std::string&                           prefix,
                          const std::chrono::system_clock::time_point& time,
                          int                                          last);

   bool LoadScan(Impl::ScanRecord& scanRecord);
   std::tuple<bool, size_t, size_t> ListObjects();


   std::string                        radarSite_;
   std::string                        bucketName_;
   std::string                        region_;
   std::shared_ptr<Aws::S3::S3Client> client_;

   std::mutex refreshMutex_;

   std::unordered_map<std::string, std::chrono::system_clock::time_point>
                                         scanTimes_;
   ScanRecord                            lastScan_;
   ScanRecord                            currentScan_;
   std::shared_mutex                     scansMutex_;
   std::chrono::system_clock::time_point lastTimeListed_;

   std::chrono::seconds updatePeriod_;

   AwsLevel2ChunksDataProvider* self_;
};

AwsLevel2ChunksDataProvider::AwsLevel2ChunksDataProvider(
   const std::string& radarSite) :
    AwsLevel2ChunksDataProvider(radarSite, kDefaultBucketName_, kDefaultRegion_)
{
}

AwsLevel2ChunksDataProvider::AwsLevel2ChunksDataProvider(
   const std::string& radarSite,
   const std::string& bucketName,
   const std::string& region) :
    p(std::make_unique<Impl>(this, radarSite, bucketName, region))
{
}

AwsLevel2ChunksDataProvider::~AwsLevel2ChunksDataProvider() = default;

std::chrono::system_clock::time_point
AwsLevel2ChunksDataProvider::GetTimePointByKey(const std::string& key) const
{
   std::chrono::system_clock::time_point time {};

   const size_t lastSeparator = key.rfind('/');
   const size_t offset =
      (lastSeparator == std::string::npos) ? 0 : lastSeparator + 1;

   // Filename format is YYYYMMDD-TTTTTT-AAA-B
   static const size_t formatSize = std::string("YYYYMMDD-TTTTTT").size();

   if (key.size() >= offset + formatSize)
   {
      using namespace std::chrono;

#if (__cpp_lib_chrono < 201907L)
      using namespace date;
#endif

      static const std::string timeFormat {"%Y%m%d-%H%M%S"};

      std::string        timeStr {key.substr(offset, formatSize)};
      std::istringstream in {timeStr};
      in >> parse(timeFormat, time);

      if (in.fail())
      {
         logger_->warn("Invalid time: \"{}\"", timeStr);
      }
   }
   else
   {
      logger_->warn("Time not parsable from key: \"{}\"", key);
   }

   return time;
}

size_t AwsLevel2ChunksDataProvider::cache_size() const
{
   return 2;
}

std::chrono::system_clock::time_point
AwsLevel2ChunksDataProvider::last_modified() const
{
   if (p->currentScan_.valid_ && p->currentScan_.lastModified_ !=
                                    std::chrono::system_clock::time_point {})
   {
      return p->currentScan_.lastModified_;
   }
   else if (p->lastScan_.valid_ && p->lastScan_.lastModified_ !=
                                      std::chrono::system_clock::time_point {})
   {
      return p->lastScan_.lastModified_;
   }
   else
   {
      return {};
   }
}
std::chrono::seconds AwsLevel2ChunksDataProvider::update_period() const
{
   std::shared_lock lock(p->scansMutex_);
   // Add an extra second of delay
   static const auto extra = std::chrono::seconds(2);
   // get update period from time between chunks
   if (p->currentScan_.valid_ && p->currentScan_.nextFile_ > 2)
   {
      auto delta =
         p->currentScan_.lastModified_ - p->currentScan_.secondLastModified_;
      return std::chrono::duration_cast<std::chrono::seconds>(delta) + extra;
   }
   else if (p->lastScan_.valid_ && p->lastScan_.nextFile_ > 2)
   {
      auto delta =
         p->lastScan_.lastModified_ - p->lastScan_.secondLastModified_;
      return std::chrono::duration_cast<std::chrono::seconds>(delta) + extra;
   }

   // default to a set update period
   return p->updatePeriod_;
}

std::string
AwsLevel2ChunksDataProvider::FindKey(std::chrono::system_clock::time_point time)
{
   logger_->debug("FindKey: {}", util::TimeString(time));

   std::shared_lock lock(p->scansMutex_);
   if (p->currentScan_.valid_ && time >= p->currentScan_.time_)
   {
      return p->currentScan_.prefix_;
   }
   else if (p->lastScan_.valid_ && time >= p->lastScan_.time_)
   {
      return p->lastScan_.prefix_;
   }

   return {};
}

std::string AwsLevel2ChunksDataProvider::FindLatestKey()
{
   std::shared_lock lock(p->scansMutex_);
   if (!p->currentScan_.valid_)
   {
      return "";
   }

   return p->currentScan_.prefix_;
}

std::chrono::system_clock::time_point
AwsLevel2ChunksDataProvider::FindLatestTime()
{
   std::shared_lock lock(p->scansMutex_);
   if (!p->currentScan_.valid_)
   {
      return {};
   }

   return p->currentScan_.time_;
}

std::vector<std::chrono::system_clock::time_point>
AwsLevel2ChunksDataProvider::GetTimePointsByDate(
   std::chrono::system_clock::time_point /*date*/)
{
   return {};
}

std::chrono::system_clock::time_point
AwsLevel2ChunksDataProvider::Impl::GetScanTime(const std::string& prefix)
{
   using namespace std::chrono;
   const auto& scanTimeIt = scanTimes_.find(prefix); // O(log(n))
   if (scanTimeIt != scanTimes_.cend())
   {
      // If the time is greater than 2 hours ago, it may be a new scan
      auto replaceBy = system_clock::now() - hours {2};
      if (scanTimeIt->second > replaceBy)
      {
         return scanTimeIt->second;
      }
   }

   Aws::S3::Model::ListObjectsV2Request request;
   request.SetBucket(bucketName_);
   request.SetPrefix(prefix);
   request.SetDelimiter("/");
   request.SetMaxKeys(1);

   auto outcome = client_->ListObjectsV2(request);
   if (outcome.IsSuccess())
   {
      auto timePoint = self_->GetTimePointByKey(
         outcome.GetResult().GetContents().at(0).GetKey());
      scanTimes_.insert_or_assign(prefix, timePoint);
      return timePoint;
   }

   return {};
}

std::string AwsLevel2ChunksDataProvider::Impl::GetScanKey(
   const std::string&                           prefix,
   const std::chrono::system_clock::time_point& time,
   int                                          last)
{

   static const std::string timeFormat {"%Y%m%d-%H%M%S"};

   // TODO
   return fmt::format(
      "{0}/{1:%Y%m%d-%H%M%S}-{2}", prefix, fmt::gmtime(time), last - 1);
}

std::tuple<bool, size_t, size_t>
AwsLevel2ChunksDataProvider::Impl::ListObjects()
{
   size_t newObjects   = 0;
   size_t totalObjects = 0;

   std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

   if (currentScan_.valid_ && !currentScan_.hasAllFiles_ &&
       lastTimeListed_ + std::chrono::minutes(2) > now)
   {
      return {true, newObjects, totalObjects};
   }
   logger_->debug("ListObjects");
   lastTimeListed_ = now;

   const std::string prefix = radarSite_ + "/";

   Aws::S3::Model::ListObjectsV2Request request;
   request.SetBucket(bucketName_);
   request.SetPrefix(prefix);
   request.SetDelimiter("/");

   auto outcome = client_->ListObjectsV2(request);

   if (outcome.IsSuccess())
   {
      auto& scans = outcome.GetResult().GetCommonPrefixes();
      logger_->debug("Found {} scans", scans.size());

      if (scans.size() > 0)
      {
         // find latest scan
         auto scanNumberMap = std::map<int, std::string>();

         for (auto& scan : scans) // O(n log(n)) n <= 999
         {
            const std::string& scanPrefix = scan.GetPrefix();
            scanNumberMap.insert_or_assign(GetScanNumber(scanPrefix),
                                           scanPrefix);
         }

         int lastScanNumber = -1;
         // Start with last scan
         int previousScanNumber = scanNumberMap.crbegin()->first;
         const int firstScanNumber = scanNumberMap.cbegin()->first;

         // Look for a gap in scan numbers. This indicates that is the latest
         // scan.

         // This indicates that highest number scan is the last scan
         // (including if there is only 1 scan)
         // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
         if (previousScanNumber != 999 || scans.size() == 1)
         {
            lastScanNumber = previousScanNumber;
         }
         else
         {
            // Have already checked scan with highest number, so skip first
            previousScanNumber = firstScanNumber;
            bool first = true;
            for (const auto& scan : scanNumberMap)
            {
               if (first)
               {
                  first = false;
                  continue;
               }
               if (scan.first != previousScanNumber + 1)
               {
                  lastScanNumber = previousScanNumber;
                  break;
               }
               previousScanNumber = scan.first;
            }
         }

         if (lastScanNumber == -1)
         {
            // 999 is the last scan
            if (firstScanNumber != 1)
            {
               lastScanNumber = previousScanNumber;
            }
            else
            {
               logger_->warn("Could not find last scan");
               // TODO make sure this makes sence
               return {false, 0, 0};
            }
         }

         std::string& lastScanPrefix = scanNumberMap.at(lastScanNumber);
         int          secondLastScanNumber =
            lastScanNumber == 1 ? 999 : lastScanNumber - 1;

         const auto& secondLastScanPrefix =
            scanNumberMap.find(secondLastScanNumber);

         if (!currentScan_.valid_ ||
             currentScan_.prefix_ != lastScanPrefix)
         {
            if (currentScan_.valid_ &&
                (secondLastScanPrefix == scanNumberMap.cend() ||
                 currentScan_.prefix_ == secondLastScanPrefix->second))
            {
               lastScan_ = currentScan_;
            }
            else if (secondLastScanPrefix != scanNumberMap.cend())
            {
               lastScan_.valid_      = true;
               lastScan_.prefix_     = secondLastScanPrefix->second;
               lastScan_.nexradFile_ = nullptr;
               lastScan_.time_ = GetScanTime(secondLastScanPrefix->second);
               lastScan_.lastModified_       = {};
               lastScan_.secondLastModified_ = {};
               lastScan_.lastKey_            = "";
               lastScan_.nextFile_           = 1;
               lastScan_.hasAllFiles_        = false;
               newObjects += 1;
            }

            currentScan_.valid_        = true;
            currentScan_.prefix_       = lastScanPrefix;
            currentScan_.nexradFile_   = nullptr;
            currentScan_.time_               = GetScanTime(lastScanPrefix);
            currentScan_.lastModified_       = {};
            currentScan_.secondLastModified_ = {};
            currentScan_.lastKey_            = "";
            currentScan_.nextFile_           = 1;
            currentScan_.hasAllFiles_        = false;
            newObjects += 1;
         }
      }
   }

   return {true, newObjects, totalObjects};
}

std::tuple<bool, size_t, size_t>
AwsLevel2ChunksDataProvider::ListObjects(std::chrono::system_clock::time_point)
{
   return {true, 0, 0};
}

std::shared_ptr<wsr88d::NexradFile>
AwsLevel2ChunksDataProvider::LoadObjectByKey(const std::string& /*prefix*/)
{
   return nullptr;
}

bool AwsLevel2ChunksDataProvider::Impl::LoadScan(Impl::ScanRecord& scanRecord)
{
   if (!scanRecord.valid_)
   {
      logger_->warn("Tried to load scan which was not listed yet");
      return false;
   }
   else if (scanRecord.hasAllFiles_)
   {
      return false;
   }

   Aws::S3::Model::ListObjectsV2Request listRequest;
   listRequest.SetBucket(bucketName_);
   listRequest.SetPrefix(scanRecord.prefix_);
   listRequest.SetDelimiter("/");
   if (!scanRecord.lastKey_.empty())
   {
      listRequest.SetStartAfter(scanRecord.lastKey_);
   }

   auto listOutcome = client_->ListObjectsV2(listRequest);
   if (!listOutcome.IsSuccess())
   {
      logger_->warn("Could not find scan at {}", scanRecord.prefix_);
      return false;
   }

   bool hasNew = false;
   auto& chunks = listOutcome.GetResult().GetContents();
   logger_->debug("Found {} new chunks.", chunks.size());
   for (const auto& chunk : chunks)
   {
      const std::string& key = chunk.GetKey();

      // KIND/585/20250324-134727-001-S
      // KIND/5/20250324-134727-001-S
      static const size_t firstSlash = std::string("KIND/").size();
      const size_t secondSlash = key.find('/', firstSlash);
      static const size_t startNumberPosOffset =
         std::string("/20250324-134727-").size();
      const size_t startNumberPos =
          secondSlash + startNumberPosOffset;
      const std::string& keyNumberStr = key.substr(startNumberPos, 3);
      const int          keyNumber    = std::stoi(keyNumberStr);
      if (keyNumber != scanRecord.nextFile_)
      {
         logger_->warn("Chunk found that was not in order {} {} {}",
                       key,
                       scanRecord.nextFile_,
                       keyNumber);
         continue;
      }

      // Now we want the ending char
      // KIND/585/20250324-134727-001-S
      static const size_t charPos =
         std::string("/20250324-134727-001-").size();
      if (secondSlash + charPos >= key.size())
      {
         logger_->warn("Chunk key was not long enough");
         continue;
      }
      const char keyChar = key[secondSlash + charPos];

      Aws::S3::Model::GetObjectRequest objectRequest;
      objectRequest.SetBucket(bucketName_);
      objectRequest.SetKey(key);

      auto outcome = client_->GetObject(objectRequest);

      if (!outcome.IsSuccess())
      {
         logger_->warn("Could not get object: {}",
                       outcome.GetError().GetMessage());
         return hasNew;
      }

      auto& body = outcome.GetResultWithOwnership().GetBody();

      switch (keyChar)
      {
      case 'S':
      { // First chunk
         scanRecord.nexradFile_ = std::make_shared<wsr88d::Ar2vFile>();
         if (!scanRecord.nexradFile_->LoadData(body))
         {
            logger_->warn("Failed to load first chunk");
            return hasNew;
         }
         break;
      }
      case 'I':
      { // Middle chunk
         if (!scanRecord.nexradFile_->LoadLDMRecords(body))
         {
            logger_->warn("Failed to load middle chunk");
            return hasNew;
         }
         break;
      }
      case 'E':
      { // Last chunk
         if (!scanRecord.nexradFile_->LoadLDMRecords(body))
         {
            logger_->warn("Failed to load last chunk");
            return hasNew;
         }
         scanRecord.hasAllFiles_ = true;
         break;
      }
      default:
         logger_->warn("Could not load chunk with unknown char");
         return hasNew;
      }
      hasNew = true;

      std::chrono::seconds lastModifiedSeconds {
         outcome.GetResult().GetLastModified().Seconds()};
      std::chrono::system_clock::time_point lastModified {lastModifiedSeconds};

      scanRecord.secondLastModified_ = scanRecord.lastModified_;
      scanRecord.lastModified_ = lastModified;

      scanRecord.nextFile_ += 1;
      scanRecord.lastKey_ = key;
   }

   if (scanRecord.nexradFile_ == nullptr)
   {
      logger_->warn("Could not load file");
   }
   else if (hasNew)
   {
      scanRecord.nexradFile_->IndexFile();
   }

   return hasNew;
}

std::shared_ptr<wsr88d::NexradFile>
AwsLevel2ChunksDataProvider::LoadObjectByTime(
   std::chrono::system_clock::time_point time)
{
   std::unique_lock lock(p->scansMutex_);

   if (p->currentScan_.valid_ && time >= p->currentScan_.time_)
   {
      return p->currentScan_.nexradFile_;
   }
   else if (p->lastScan_.valid_ && time >= p->lastScan_.time_)
   {
      return p->lastScan_.nexradFile_;
   }
   else
   {
      logger_->warn("Could not find scan with time");
      return nullptr;
   }
}

std::shared_ptr<wsr88d::NexradFile>
AwsLevel2ChunksDataProvider::LoadLatestObject()
{
   return p->currentScan_.nexradFile_;
}

std::shared_ptr<wsr88d::NexradFile>
AwsLevel2ChunksDataProvider::LoadSecondLatestObject()
{
   return p->lastScan_.nexradFile_;
}

int AwsLevel2ChunksDataProvider::Impl::GetScanNumber(const std::string& prefix)
{
   // KIND/585/20250324-134727-001-S
   static const size_t firstSlash      = std::string("KIND/").size();
   const std::string&  prefixNumberStr = prefix.substr(firstSlash, 3);
   return std::stoi(prefixNumberStr);
}

std::pair<size_t, size_t> AwsLevel2ChunksDataProvider::Refresh()
{
   using namespace std::chrono;

   boost::timer::cpu_timer timer {};
   timer.start();

   std::unique_lock lock(p->refreshMutex_);
   std::unique_lock scanLock(p->scansMutex_);

   auto [success, newObjects, totalObjects] = p->ListObjects();

   if (p->currentScan_.valid_)
   {
      if (p->LoadScan(p->currentScan_))
      {
         newObjects += 1;
      }
      totalObjects += 1;
   }
   if (p->lastScan_.valid_)
   {
      /*
      if (p->LoadScan(p->lastScan_))
      {
         newObjects += 1;
      }
      */
      totalObjects += 1;
   }

   timer.stop();
   logger_->debug("Refresh() in {}", timer.format(6, "%ws"));
   return std::make_pair(newObjects, totalObjects);
}

void AwsLevel2ChunksDataProvider::RequestAvailableProducts() {}
std::vector<std::string> AwsLevel2ChunksDataProvider::GetAvailableProducts()
{
   return {};
}

AwsLevel2ChunksDataProvider::AwsLevel2ChunksDataProvider(
   AwsLevel2ChunksDataProvider&&) noexcept = default;
AwsLevel2ChunksDataProvider& AwsLevel2ChunksDataProvider::operator=(
   AwsLevel2ChunksDataProvider&&) noexcept = default;

} // namespace scwx::provider
