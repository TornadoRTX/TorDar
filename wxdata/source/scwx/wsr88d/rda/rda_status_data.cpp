#include <scwx/wsr88d/rda/rda_status_data.hpp>
#include <scwx/util/logger.hpp>

namespace scwx
{
namespace wsr88d
{
namespace rda
{

static const std::string logPrefix_ = "scwx::wsr88d::rda::rda_status_data";
static const auto        logger_    = util::Logger::Create(logPrefix_);

class RdaStatusDataImpl
{
public:
   explicit RdaStatusDataImpl() = default;
   ~RdaStatusDataImpl()         = default;

   uint16_t                 rdaStatus_ {0};
   uint16_t                 operabilityStatus_ {0};
   uint16_t                 controlStatus_ {0};
   uint16_t                 auxiliaryPowerGeneratorState_ {0};
   uint16_t                 averageTransmitterPower_ {0};
   int16_t                  horizontalReflectivityCalibrationCorrection_ {0};
   uint16_t                 dataTransmissionEnabled_ {0};
   uint16_t                 volumeCoveragePatternNumber_ {0};
   uint16_t                 rdaControlAuthorization_ {0};
   uint16_t                 rdaBuildNumber_ {0};
   uint16_t                 operationalMode_ {0};
   uint16_t                 superResolutionStatus_ {0};
   uint16_t                 clutterMitigationDecisionStatus_ {0};
   uint16_t                 rdaScanAndDataFlags_ {0};
   uint16_t                 rdaAlarmSummary_ {0};
   uint16_t                 commandAcknowledgement_ {0};
   uint16_t                 channelControlStatus_ {0};
   uint16_t                 spotBlankingStatus_ {0};
   uint16_t                 bypassMapGenerationDate_ {0};
   uint16_t                 bypassMapGenerationTime_ {0};
   uint16_t                 clutterFilterMapGenerationDate_ {0};
   uint16_t                 clutterFilterMapGenerationTime_ {0};
   int16_t                  verticalReflectivityCalibrationCorrection_ {0};
   uint16_t                 transitionPowerSourceStatus_ {0};
   uint16_t                 rmsControlStatus_ {0};
   uint16_t                 performanceCheckStatus_ {0};
   std::array<uint16_t, 14> alarmCodes_ {0};
   uint16_t                 signalProcessingOptions_ {0};
   uint16_t                 downloadedPatternNumber_ {0};
   uint16_t                 statusVersion_ {0};
};

RdaStatusData::RdaStatusData() :
    Level2Message(), p(std::make_unique<RdaStatusDataImpl>())
{
}
RdaStatusData::~RdaStatusData() = default;

RdaStatusData::RdaStatusData(RdaStatusData&&) noexcept            = default;
RdaStatusData& RdaStatusData::operator=(RdaStatusData&&) noexcept = default;

uint16_t RdaStatusData::rda_status() const
{
   return p->rdaStatus_;
}

uint16_t RdaStatusData::operability_status() const
{
   return p->operabilityStatus_;
}

uint16_t RdaStatusData::control_status() const
{
   return p->controlStatus_;
}

uint16_t RdaStatusData::auxiliary_power_generator_state() const
{
   return p->auxiliaryPowerGeneratorState_;
}

uint16_t RdaStatusData::average_transmitter_power() const
{
   return p->averageTransmitterPower_;
}

float RdaStatusData::horizontal_reflectivity_calibration_correction() const
{
   return p->horizontalReflectivityCalibrationCorrection_ * 0.01f;
}

uint16_t RdaStatusData::data_transmission_enabled() const
{
   return p->dataTransmissionEnabled_;
}

uint16_t RdaStatusData::volume_coverage_pattern_number() const
{
   return p->volumeCoveragePatternNumber_;
}

uint16_t RdaStatusData::rda_control_authorization() const
{
   return p->rdaControlAuthorization_;
}

uint16_t RdaStatusData::rda_build_number() const
{
   return p->rdaBuildNumber_;
}

uint16_t RdaStatusData::operational_mode() const
{
   return p->operationalMode_;
}

uint16_t RdaStatusData::super_resolution_status() const
{
   return p->superResolutionStatus_;
}

uint16_t RdaStatusData::clutter_mitigation_decision_status() const
{
   return p->clutterMitigationDecisionStatus_;
}

uint16_t RdaStatusData::rda_scan_and_data_flags() const
{
   return p->rdaScanAndDataFlags_;
}

uint16_t RdaStatusData::rda_alarm_summary() const
{
   return p->rdaAlarmSummary_;
}

uint16_t RdaStatusData::command_acknowledgement() const
{
   return p->commandAcknowledgement_;
}

uint16_t RdaStatusData::channel_control_status() const
{
   return p->channelControlStatus_;
}

uint16_t RdaStatusData::spot_blanking_status() const
{
   return p->spotBlankingStatus_;
}

uint16_t RdaStatusData::bypass_map_generation_date() const
{
   return p->bypassMapGenerationDate_;
}

uint16_t RdaStatusData::bypass_map_generation_time() const
{
   return p->bypassMapGenerationTime_;
}

uint16_t RdaStatusData::clutter_filter_map_generation_date() const
{
   return p->clutterFilterMapGenerationDate_;
}

uint16_t RdaStatusData::clutter_filter_map_generation_time() const
{
   return p->clutterFilterMapGenerationTime_;
}

float RdaStatusData::vertical_reflectivity_calibration_correction() const
{
   return p->verticalReflectivityCalibrationCorrection_ * 0.01f;
}

uint16_t RdaStatusData::transition_power_source_status() const
{
   return p->transitionPowerSourceStatus_;
}

uint16_t RdaStatusData::rms_control_status() const
{
   return p->rmsControlStatus_;
}

uint16_t RdaStatusData::performance_check_status() const
{
   return p->performanceCheckStatus_;
}

uint16_t RdaStatusData::alarm_codes(unsigned i) const
{
   return p->alarmCodes_[i];
}

uint16_t RdaStatusData::signal_processing_options() const
{
   return p->signalProcessingOptions_;
}

uint16_t RdaStatusData::downloaded_pattern_number() const
{
   return p->downloadedPatternNumber_;
}

uint16_t RdaStatusData::status_version() const
{
   return p->statusVersion_;
}

bool RdaStatusData::Parse(std::istream& is)
{
   logger_->trace("Parsing RDA Status Data (Message Type 2)");

   bool   messageValid = true;
   size_t bytesRead    = 0;

   is.read(reinterpret_cast<char*>(&p->rdaStatus_), 2);                    // 1
   is.read(reinterpret_cast<char*>(&p->operabilityStatus_), 2);            // 2
   is.read(reinterpret_cast<char*>(&p->controlStatus_), 2);                // 3
   is.read(reinterpret_cast<char*>(&p->auxiliaryPowerGeneratorState_), 2); // 4
   is.read(reinterpret_cast<char*>(&p->averageTransmitterPower_), 2);      // 5
   is.read(
      reinterpret_cast<char*>(&p->horizontalReflectivityCalibrationCorrection_),
      2);                                                                 // 6
   is.read(reinterpret_cast<char*>(&p->dataTransmissionEnabled_), 2);     // 7
   is.read(reinterpret_cast<char*>(&p->volumeCoveragePatternNumber_), 2); // 8
   is.read(reinterpret_cast<char*>(&p->rdaControlAuthorization_), 2);     // 9
   is.read(reinterpret_cast<char*>(&p->rdaBuildNumber_), 2);              // 10
   is.read(reinterpret_cast<char*>(&p->operationalMode_), 2);             // 11
   is.read(reinterpret_cast<char*>(&p->superResolutionStatus_), 2);       // 12
   is.read(reinterpret_cast<char*>(&p->clutterMitigationDecisionStatus_),
           2);                                                        // 13
   is.read(reinterpret_cast<char*>(&p->rdaScanAndDataFlags_), 2);     // 14
   is.read(reinterpret_cast<char*>(&p->rdaAlarmSummary_), 2);         // 15
   is.read(reinterpret_cast<char*>(&p->commandAcknowledgement_), 2);  // 16
   is.read(reinterpret_cast<char*>(&p->channelControlStatus_), 2);    // 17
   is.read(reinterpret_cast<char*>(&p->spotBlankingStatus_), 2);      // 18
   is.read(reinterpret_cast<char*>(&p->bypassMapGenerationDate_), 2); // 19
   is.read(reinterpret_cast<char*>(&p->bypassMapGenerationTime_), 2); // 20
   is.read(reinterpret_cast<char*>(&p->clutterFilterMapGenerationDate_),
           2); // 21
   is.read(reinterpret_cast<char*>(&p->clutterFilterMapGenerationTime_),
           2); // 22
   is.read(
      reinterpret_cast<char*>(&p->verticalReflectivityCalibrationCorrection_),
      2);                                                                 // 23
   is.read(reinterpret_cast<char*>(&p->transitionPowerSourceStatus_), 2); // 24
   is.read(reinterpret_cast<char*>(&p->rmsControlStatus_), 2);            // 25
   is.read(reinterpret_cast<char*>(&p->performanceCheckStatus_), 2);      // 26
   is.read(reinterpret_cast<char*>(&p->alarmCodes_),
           p->alarmCodes_.size() * 2); // 27-40
   bytesRead += 80;

   p->rdaStatus_                    = ntohs(p->rdaStatus_);
   p->operabilityStatus_            = ntohs(p->operabilityStatus_);
   p->controlStatus_                = ntohs(p->controlStatus_);
   p->auxiliaryPowerGeneratorState_ = ntohs(p->auxiliaryPowerGeneratorState_);
   p->averageTransmitterPower_      = ntohs(p->averageTransmitterPower_);
   p->horizontalReflectivityCalibrationCorrection_ =
      ntohs(p->horizontalReflectivityCalibrationCorrection_);
   p->dataTransmissionEnabled_     = ntohs(p->dataTransmissionEnabled_);
   p->volumeCoveragePatternNumber_ = ntohs(p->volumeCoveragePatternNumber_);
   p->rdaControlAuthorization_     = ntohs(p->rdaControlAuthorization_);
   p->rdaBuildNumber_              = ntohs(p->rdaBuildNumber_);
   p->operationalMode_             = ntohs(p->operationalMode_);
   p->superResolutionStatus_       = ntohs(p->superResolutionStatus_);
   p->clutterMitigationDecisionStatus_ =
      ntohs(p->clutterMitigationDecisionStatus_);
   p->rdaScanAndDataFlags_     = ntohs(p->rdaScanAndDataFlags_);
   p->rdaAlarmSummary_         = ntohs(p->rdaAlarmSummary_);
   p->commandAcknowledgement_  = ntohs(p->commandAcknowledgement_);
   p->channelControlStatus_    = ntohs(p->channelControlStatus_);
   p->spotBlankingStatus_      = ntohs(p->spotBlankingStatus_);
   p->bypassMapGenerationDate_ = ntohs(p->bypassMapGenerationDate_);
   p->bypassMapGenerationTime_ = ntohs(p->bypassMapGenerationTime_);
   p->clutterFilterMapGenerationDate_ =
      ntohs(p->clutterFilterMapGenerationDate_);
   p->clutterFilterMapGenerationTime_ =
      ntohs(p->clutterFilterMapGenerationTime_);
   p->verticalReflectivityCalibrationCorrection_ =
      ntohs(p->verticalReflectivityCalibrationCorrection_);
   p->transitionPowerSourceStatus_ = ntohs(p->transitionPowerSourceStatus_);
   p->rmsControlStatus_            = ntohs(p->rmsControlStatus_);
   p->performanceCheckStatus_      = ntohs(p->performanceCheckStatus_);
   SwapArray(p->alarmCodes_);

   // RDA Build 18.0 increased the size of the message from 80 to 120 bytes
   if (header().message_size() * 2 > Level2MessageHeader::SIZE + 80)
   {
      is.read(reinterpret_cast<char*>(&p->signalProcessingOptions_), 2); // 41
      is.seekg(34, std::ios_base::cur); // 42-58
      is.read(reinterpret_cast<char*>(&p->downloadedPatternNumber_), 2); // 59
      is.read(reinterpret_cast<char*>(&p->statusVersion_), 2);           // 60
      bytesRead += 40;

      p->signalProcessingOptions_ = ntohs(p->signalProcessingOptions_);
      p->statusVersion_           = ntohs(p->statusVersion_);
   }

   if (!ValidateMessage(is, bytesRead))
   {
      messageValid = false;
   }

   return messageValid;
}

std::shared_ptr<RdaStatusData>
RdaStatusData::Create(Level2MessageHeader&& header, std::istream& is)
{
   std::shared_ptr<RdaStatusData> message = std::make_shared<RdaStatusData>();
   message->set_header(std::move(header));

   if (!message->Parse(is))
   {
      message.reset();
   }

   return message;
}

} // namespace rda
} // namespace wsr88d
} // namespace scwx
