#pragma once

#include <scwx/wsr88d/rda/level2_message.hpp>

namespace scwx::wsr88d::rda
{

class RdaStatusData : public Level2Message
{
public:
   explicit RdaStatusData();
   ~RdaStatusData();

   RdaStatusData(const RdaStatusData&)            = delete;
   RdaStatusData& operator=(const RdaStatusData&) = delete;

   RdaStatusData(RdaStatusData&&) noexcept;
   RdaStatusData& operator=(RdaStatusData&&) noexcept;

   [[nodiscard]] std::uint16_t rda_status() const;
   [[nodiscard]] std::uint16_t operability_status() const;
   [[nodiscard]] std::uint16_t control_status() const;
   [[nodiscard]] std::uint16_t auxiliary_power_generator_state() const;
   [[nodiscard]] std::uint16_t average_transmitter_power() const;
   [[nodiscard]] float horizontal_reflectivity_calibration_correction() const;
   [[nodiscard]] std::uint16_t data_transmission_enabled() const;
   [[nodiscard]] std::uint16_t volume_coverage_pattern_number() const;
   [[nodiscard]] std::uint16_t rda_control_authorization() const;
   [[nodiscard]] std::uint16_t rda_build_number() const;
   [[nodiscard]] std::uint16_t operational_mode() const;
   [[nodiscard]] std::uint16_t super_resolution_status() const;
   [[nodiscard]] std::uint16_t clutter_mitigation_decision_status() const;
   [[nodiscard]] std::uint16_t rda_scan_and_data_flags() const;
   [[nodiscard]] std::uint16_t rda_alarm_summary() const;
   [[nodiscard]] std::uint16_t command_acknowledgement() const;
   [[nodiscard]] std::uint16_t channel_control_status() const;
   [[nodiscard]] std::uint16_t spot_blanking_status() const;
   [[nodiscard]] std::uint16_t bypass_map_generation_date() const;
   [[nodiscard]] std::uint16_t bypass_map_generation_time() const;
   [[nodiscard]] std::uint16_t clutter_filter_map_generation_date() const;
   [[nodiscard]] std::uint16_t clutter_filter_map_generation_time() const;
   [[nodiscard]] float vertical_reflectivity_calibration_correction() const;
   [[nodiscard]] std::uint16_t transition_power_source_status() const;
   [[nodiscard]] std::uint16_t rms_control_status() const;
   [[nodiscard]] std::uint16_t performance_check_status() const;
   [[nodiscard]] std::uint16_t alarm_codes(unsigned i) const;
   [[nodiscard]] std::uint16_t signal_processing_options() const;
   [[nodiscard]] std::uint16_t downloaded_pattern_number() const;
   [[nodiscard]] std::uint16_t status_version() const;

   bool Parse(std::istream& is) override;

   static std::shared_ptr<RdaStatusData> Create(Level2MessageHeader&& header,
                                                std::istream&         is);

private:
   class Impl;
   std::unique_ptr<Impl> p;
};

} // namespace scwx::wsr88d::rda
