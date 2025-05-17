#include <scwx/wsr88d/rda/level2_message_factory.hpp>

#include <scwx/util/logger.hpp>
#include <scwx/util/vectorbuf.hpp>
#include <scwx/wsr88d/rda/clutter_filter_bypass_map.hpp>
#include <scwx/wsr88d/rda/clutter_filter_map.hpp>
#include <scwx/wsr88d/rda/digital_radar_data.hpp>
#include <scwx/wsr88d/rda/digital_radar_data_generic.hpp>
#include <scwx/wsr88d/rda/performance_maintenance_data.hpp>
#include <scwx/wsr88d/rda/rda_adaptation_data.hpp>
#include <scwx/wsr88d/rda/rda_prf_data.hpp>
#include <scwx/wsr88d/rda/rda_status_data.hpp>
#include <scwx/wsr88d/rda/volume_coverage_pattern_data.hpp>

#include <unordered_map>
#include <vector>

namespace scwx::wsr88d::rda
{

static const std::string logPrefix_ =
   "scwx::wsr88d::rda::level2_message_factory";
static const auto logger_ = util::Logger::Create(logPrefix_);

using CreateLevel2MessageFunction =
   std::function<std::shared_ptr<Level2Message>(Level2MessageHeader&&,
                                                std::istream&)>;

static const std::unordered_map<unsigned int, CreateLevel2MessageFunction>
   create_ {{1, DigitalRadarData::Create},
            {2, RdaStatusData::Create},
            {3, PerformanceMaintenanceData::Create},
            {5, VolumeCoveragePatternData::Create},
            {13, ClutterFilterBypassMap::Create},
            {15, ClutterFilterMap::Create},
            {18, RdaAdaptationData::Create},
            {31, DigitalRadarDataGeneric::Create},
            {32, RdaPrfData::Create}};

struct Level2MessageFactory::Context
{
   Context() :
       messageBuffer_ {messageData_}, messageBufferStream_ {&messageBuffer_}
   {
   }

   std::vector<char> messageData_ {};
   std::size_t       bufferedSize_ {};
   util::vectorbuf   messageBuffer_;
   std::istream      messageBufferStream_;
   bool              bufferingData_ {false};
};

std::shared_ptr<Level2MessageFactory::Context>
Level2MessageFactory::CreateContext()
{
   return std::make_shared<Context>();
}

Level2MessageInfo Level2MessageFactory::Create(std::istream&             is,
                                               std::shared_ptr<Context>& ctx)
{
   Level2MessageInfo   info;
   Level2MessageHeader header;
   info.headerValid  = header.Parse(is);
   info.messageValid = info.headerValid;

   std::uint16_t segment       = 0;
   std::uint16_t totalSegments = 0;
   std::size_t   dataSize      = 0;

   if (info.headerValid)
   {
      if (header.message_size() == std::numeric_limits<std::uint16_t>::max())
      {
         // A message size of 65535 indicates a message with a single segment.
         // The size is specified in the bytes normally reserved for the segment
         // number and total number of segments.
         segment       = 1;
         totalSegments = 1;
         dataSize =
            (static_cast<std::size_t>(header.number_of_message_segments())
             << 16) + // NOLINT(cppcoreguidelines-avoid-magic-numbers)
            header.message_segment_number();
      }
      else
      {
         segment       = header.message_segment_number();
         totalSegments = header.number_of_message_segments();
         dataSize      = static_cast<std::size_t>(header.message_size()) * 2 -
                    Level2MessageHeader::SIZE;
      }
   }

   if (info.headerValid && create_.find(header.message_type()) == create_.end())
   {
      logger_->warn("Unknown message type: {}",
                    static_cast<unsigned>(header.message_type()));
      info.messageValid = false;
   }

   if (info.messageValid)
   {
      std::uint8_t messageType = header.message_type();

      std::istream* messageStream = nullptr;

      if (totalSegments == 1)
      {
         logger_->trace("Found Message {}", static_cast<unsigned>(messageType));
         messageStream = &is;
      }
      else
      {
         logger_->trace("Found Message {} Segment {}/{}",
                        static_cast<unsigned>(messageType),
                        segment,
                        totalSegments);

         if (segment == 1)
         {
            // Estimate total message size
            ctx->messageData_.resize(dataSize * totalSegments);
            ctx->messageBufferStream_.clear();
            ctx->bufferedSize_  = 0;
            ctx->bufferingData_ = true;
         }
         else if (!ctx->bufferingData_)
         {
            // Segment number did not start at 1
            logger_->trace("Ignoring Segment {}/{}, did not start at 1",
                           segment,
                           totalSegments);
            info.messageValid = false;
         }

         if (ctx->bufferingData_)
         {
            if (ctx->messageData_.capacity() < ctx->bufferedSize_ + dataSize)
            {
               logger_->debug("Bad size estimate, increasing size");

               // Estimate remaining size
               static const std::uint16_t kMinRemainingSegments_ = 100u;
               const std::uint16_t remainingSegments = std::max<std::uint16_t>(
                  totalSegments - segment + 1, kMinRemainingSegments_);
               const std::size_t remainingSize = remainingSegments * dataSize;

               ctx->messageData_.resize(ctx->bufferedSize_ + remainingSize);
            }

            is.read(&ctx->messageData_[ctx->bufferedSize_],
                    static_cast<std::streamsize>(dataSize));
            ctx->bufferedSize_ += dataSize;

            if (is.eof())
            {
               logger_->warn("End of file reached trying to buffer message");
               info.messageValid = false;
               ctx->messageData_.shrink_to_fit();
               ctx->bufferedSize_  = 0;
               ctx->bufferingData_ = false;
            }
            else if (segment == totalSegments)
            {
               ctx->messageBuffer_.update_read_pointers(ctx->bufferedSize_);
               header.set_message_size(static_cast<std::uint16_t>(
                  ctx->bufferedSize_ / 2 + Level2MessageHeader::SIZE));

               messageStream = &ctx->messageBufferStream_;
            }
         }
      }

      if (messageStream != nullptr)
      {
         info.message =
            create_.at(messageType)(std::move(header), *messageStream);
         ctx->messageData_.resize(0);
         ctx->messageData_.shrink_to_fit();
         ctx->messageBufferStream_.clear();
         ctx->bufferedSize_  = 0;
         ctx->bufferingData_ = false;
      }
   }
   else if (info.headerValid)
   {
      // Seek to the end of the current message
      is.seekg(static_cast<std::streamoff>(dataSize), std::ios_base::cur);
   }

   if (info.message == nullptr)
   {
      info.messageValid = false;
   }

   return info;
}

} // namespace scwx::wsr88d::rda
