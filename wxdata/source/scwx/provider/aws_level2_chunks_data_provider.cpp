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
      explicit ScanRecord(std::string prefix) :
          prefix_ {std::move(prefix)},
          nexradFile_ {},
          lastModified_ {},
          secondLastModified_ {}
      {
      }
      ~ScanRecord()                            = default;
      ScanRecord(const ScanRecord&)            = default;
      ScanRecord(ScanRecord&&)                 = default;
      ScanRecord& operator=(const ScanRecord&) = default;
      ScanRecord& operator=(ScanRecord&&)      = default;

      std::string                           prefix_;
      std::shared_ptr<wsr88d::Ar2vFile>     nexradFile_;
      std::chrono::system_clock::time_point lastModified_;
      std::chrono::system_clock::time_point secondLastModified_;
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
       scans_ {},
       scansMutex_ {},
       lastModified_ {},
       updatePeriod_ {},
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
   std::string                           GetScanKey(const std::string&                           prefix,
                                                    const std::chrono::system_clock::time_point& time,
                                                    int                                          last);
   std::shared_ptr<wsr88d::NexradFile>   LoadScan(Impl::ScanRecord& scanRecord);

   std::string                        radarSite_;
   std::string                        bucketName_;
   std::string                        region_;
   std::shared_ptr<Aws::S3::S3Client> client_;

   std::mutex refreshMutex_;

   std::map<std::chrono::system_clock::time_point, ScanRecord> scans_;
   std::shared_mutex                                           scansMutex_;

   std::chrono::system_clock::time_point lastModified_;
   std::chrono::seconds                  updatePeriod_;

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
   return p->scans_.size();
}

std::chrono::system_clock::time_point
AwsLevel2ChunksDataProvider::last_modified() const
{
   return p->lastModified_;
}
std::chrono::seconds AwsLevel2ChunksDataProvider::update_period() const
{
   return p->updatePeriod_;
}

std::string
AwsLevel2ChunksDataProvider::FindKey(std::chrono::system_clock::time_point time)
{
   logger_->debug("FindKey: {}", util::TimeString(time));

   std::shared_lock lock(p->scansMutex_);

   auto element = util::GetBoundedElement(p->scans_, time);

   if (element.has_value())
   {
      return element->prefix_;
   }

   return {};
}

std::string AwsLevel2ChunksDataProvider::FindLatestKey()
{
   std::shared_lock lock(p->scansMutex_);
   if (p->scans_.empty())
   {
      return "";
   }

   return p->scans_.crbegin()->second.prefix_;
}

std::chrono::system_clock::time_point
AwsLevel2ChunksDataProvider::FindLatestTime()
{
   std::shared_lock lock(p->scansMutex_);
   if (p->scans_.empty())
   {
      return {};
   }

   return p->scans_.crbegin()->first;
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
   Aws::S3::Model::ListObjectsV2Request request;
   request.SetBucket(bucketName_);
   request.SetPrefix(prefix);
   request.SetDelimiter("/");
   request.SetMaxKeys(1);

   auto outcome = client_->ListObjectsV2(request);
   if (outcome.IsSuccess())
   {
      return self_->GetTimePointByKey(
         outcome.GetResult().GetContents().at(0).GetKey());
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
AwsLevel2ChunksDataProvider::ListObjects(std::chrono::system_clock::time_point)
{
   // TODO this is slow. It could probably be speed up by not reloading every
   // scan every time.
   const std::string prefix = p->radarSite_ + "/";

   logger_->debug("ListObjects: {}", prefix);

   Aws::S3::Model::ListObjectsV2Request request;
   request.SetBucket(p->bucketName_);
   request.SetPrefix(prefix);
   request.SetDelimiter("/");

   auto outcome = p->client_->ListObjectsV2(request);

   size_t newObjects   = 0;
   size_t totalObjects = 0;

   if (outcome.IsSuccess())
   {
      auto& scans = outcome.GetResult().GetCommonPrefixes();
      logger_->debug("Found {} scans", scans.size());

      for (const auto& scan : scans)
      {
         const std::string& prefixScan = scan.GetPrefix();

         auto time = p->GetScanTime(prefixScan);

         if (!p->scans_.contains(time))
         {
            p->scans_.insert_or_assign(time, Impl::ScanRecord {prefixScan});
            newObjects++;
         }

         totalObjects++;
      }
   }

   return {outcome.IsSuccess(), newObjects, totalObjects};
}

std::shared_ptr<wsr88d::NexradFile>
AwsLevel2ChunksDataProvider::LoadObjectByKey(const std::string& /*prefix*/)
{
   return nullptr;
}

std::shared_ptr<wsr88d::NexradFile>
AwsLevel2ChunksDataProvider::Impl::LoadScan(Impl::ScanRecord& scanRecord)
{
   if (scanRecord.hasAllFiles_)
   {
      return scanRecord.nexradFile_;
   }

   // TODO can get  only new records using scanRecords last
   Aws::S3::Model::ListObjectsV2Request listRequest;
   listRequest.SetBucket(bucketName_);
   listRequest.SetPrefix(scanRecord.prefix_);
   listRequest.SetDelimiter("/");

   auto listOutcome = client_->ListObjectsV2(listRequest);
   if (!listOutcome.IsSuccess())
   {
      logger_->warn("Could not find scan at {}", scanRecord.prefix_);
      return nullptr;
   }

   auto& chunks = listOutcome.GetResult().GetContents();
   for (const auto& chunk : chunks)
   {
      const std::string& key = chunk.GetKey();

      // We just want the number of this chunk for now
      // KIND/585/20250324-134727-001-S
      static const size_t startNumberPos =
         std::string("KIND/585/20250324-134727-").size();
      const std::string& keyNumberStr = key.substr(startNumberPos, 3);
      const int          keyNumber    = std::stoi(keyNumberStr);
      if (keyNumber != scanRecord.nextFile_)
      {
         continue;
      }

      // Now we want the ending char
      // KIND/585/20250324-134727-001-S
      static const size_t charPos =
         std::string("KIND/585/20250324-134727-001-").size();
      const char keyChar = key[charPos];

      Aws::S3::Model::GetObjectRequest objectRequest;
      objectRequest.SetBucket(bucketName_);
      objectRequest.SetKey(key);

      auto outcome = client_->GetObject(objectRequest);

      if (!outcome.IsSuccess())
      {
         logger_->warn("Could not get object: {}",
                       outcome.GetError().GetMessage());
         return nullptr;
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
            return nullptr;
         }
         break;
      }
      case 'I':
      { // Middle chunk
         if (!scanRecord.nexradFile_->LoadLDMRecords(body))
         {
            logger_->warn("Failed to load middle chunk");
            return nullptr;
         }
         break;
      }
      case 'E':
      { // Last chunk
         if (!scanRecord.nexradFile_->LoadLDMRecords(body))
         {
            logger_->warn("Failed to load last chunk");
            return nullptr;
         }
         scanRecord.hasAllFiles_ = true;
         break;
      }
      default:
         return nullptr;
      }

      std::chrono::seconds lastModifiedSeconds {
         outcome.GetResult().GetLastModified().Seconds()};
      std::chrono::system_clock::time_point lastModified {lastModifiedSeconds};

      scanRecord.secondLastModified_ = scanRecord.lastModified_;
      scanRecord.lastModified_       = lastModified;

      scanRecord.nextFile_ += 1;
   }
   scanRecord.nexradFile_->IndexFile();

   if (!scans_.empty())
   {
      auto& lastScan = scans_.crend()->second;
      lastModified_  = lastScan.lastModified_;
      if (lastScan.secondLastModified_ !=
          std::chrono::system_clock::time_point())
      {
         auto delta = lastScan.lastModified_ - lastScan.secondLastModified_;
         updatePeriod_ =
            std::chrono::duration_cast<std::chrono::seconds>(delta);
      }
   }

   return scanRecord.nexradFile_;
}

std::shared_ptr<wsr88d::NexradFile>
AwsLevel2ChunksDataProvider::LoadObjectByTime(
   std::chrono::system_clock::time_point time)
{
   std::shared_lock lock(p->scansMutex_);

   logger_->error("LoadObjectByTime({})", time);

   auto scanRecord = util::GetBoundedElementPointer(p->scans_, time);
   if (scanRecord == nullptr)
   {
      logger_->warn("Could not find object at time {}", time);
      return nullptr;
   }

   // The scanRecord must be a reference
   return p->LoadScan(p->scans_.at(scanRecord->first));
}

std::shared_ptr<wsr88d::NexradFile>
AwsLevel2ChunksDataProvider::LoadLatestObject()
{
   return LoadObjectByTime(FindLatestTime());
}

std::pair<size_t, size_t> AwsLevel2ChunksDataProvider::Refresh()
{
   using namespace std::chrono;

   std::unique_lock lock(p->refreshMutex_);

   auto [success, newObjects, totalObjects] = ListObjects({});

   for (auto& scanRecord : p->scans_)
   {
      if (scanRecord.second.nexradFile_ != nullptr)
      {
         p->LoadScan(scanRecord.second);
         newObjects += 1;
      }
   }

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
