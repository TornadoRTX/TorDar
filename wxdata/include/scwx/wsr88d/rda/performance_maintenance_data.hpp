#pragma once

#include <scwx/wsr88d/rda/level2_message.hpp>

namespace scwx
{
namespace wsr88d
{
namespace rda
{

class PerformanceMaintenanceDataImpl;

class PerformanceMaintenanceData : public Level2Message
{
public:
   explicit PerformanceMaintenanceData();
   ~PerformanceMaintenanceData();

   PerformanceMaintenanceData(const PerformanceMaintenanceData&) = delete;
   PerformanceMaintenanceData&
   operator=(const PerformanceMaintenanceData&) = delete;

   PerformanceMaintenanceData(PerformanceMaintenanceData&&) noexcept;
   PerformanceMaintenanceData& operator=(PerformanceMaintenanceData&&) noexcept;

   uint16_t    loop_back_set_status() const;
   uint32_t    t1_output_frames() const;
   uint32_t    t1_input_frames() const;
   uint32_t    router_memory_used() const;
   uint32_t    router_memory_free() const;
   uint16_t    router_memory_utilization() const;
   uint16_t    route_to_rpg() const;
   uint16_t    t1_port_status() const;
   uint16_t    router_dedicated_ethernet_port_status() const;
   uint16_t    router_commercial_ethernet_port_status() const;
   uint32_t    csu_24hr_errored_seconds() const;
   uint32_t    csu_24hr_severely_errored_seconds() const;
   uint32_t    csu_24hr_severely_errored_framing_seconds() const;
   uint32_t    csu_24hr_unavailable_seconds() const;
   uint32_t    csu_24hr_controlled_slip_seconds() const;
   uint32_t    csu_24hr_path_coding_violations() const;
   uint32_t    csu_24hr_line_errored_seconds() const;
   uint32_t    csu_24hr_bursty_errored_seconds() const;
   uint32_t    csu_24hr_degraded_minutes() const;
   uint32_t    lan_switch_cpu_utilization() const;
   uint16_t    lan_switch_memory_utilization() const;
   uint16_t    ifdr_chasis_temperature() const;
   uint16_t    ifdr_fpga_temperature() const;
   uint16_t    ntp_status() const;
   uint16_t    ipc_status() const;
   uint16_t    commanded_channel_control() const;
   uint16_t    polarization() const;
   float       ame_internal_temperature() const;
   float       ame_receiver_module_temperature() const;
   float       ame_bite_cal_module_temperature() const;
   uint16_t    ame_peltier_pulse_width_modulation() const;
   uint16_t    ame_peltier_status() const;
   uint16_t    ame_a_d_converter_status() const;
   uint16_t    ame_state() const;
   float       ame_3_3v_ps_voltage() const;
   float       ame_5v_ps_voltage() const;
   float       ame_6_5v_ps_voltage() const;
   float       ame_15v_ps_voltage() const;
   float       ame_48v_ps_voltage() const;
   float       ame_stalo_power() const;
   float       peltier_current() const;
   float       adc_calibration_reference_voltage() const;
   uint16_t    ame_mode() const;
   uint16_t    ame_peltier_mode() const;
   float       ame_peltier_inside_fan_current() const;
   float       ame_peltier_outside_fan_current() const;
   float       horizontal_tr_limiter_voltage() const;
   float       vertical_tr_limiter_voltage() const;
   float       adc_calibration_offset_voltage() const;
   float       adc_calibration_gain_correction() const;
   uint16_t    rcp_status() const;
   std::string rcp_string() const;
   uint16_t    spip_power_buttons() const;
   float       master_power_administrator_load() const;
   float       expansion_power_administrator_load() const;
   uint16_t    _5vdc_ps() const;
   uint16_t    _15vdc_ps() const;
   uint16_t    _28vdc_ps() const;
   uint16_t    neg_15vdc_ps() const;
   uint16_t    _45vdc_ps() const;
   uint16_t    filament_ps_voltage() const;
   uint16_t    vacuum_pump_ps_voltage() const;
   uint16_t    focus_coil_ps_voltage() const;
   uint16_t    filament_ps() const;
   uint16_t    klystron_warmup() const;
   uint16_t    transmitter_available() const;
   uint16_t    wg_switch_position() const;
   uint16_t    wg_pfn_transfer_interlock() const;
   uint16_t    maintenance_mode() const;
   uint16_t    maintenance_required() const;
   uint16_t    pfn_switch_position() const;
   uint16_t    modulator_overload() const;
   uint16_t    modulator_inv_current() const;
   uint16_t    modulator_switch_fail() const;
   uint16_t    main_power_voltage() const;
   uint16_t    charging_system_fail() const;
   uint16_t    inverse_diode_current() const;
   uint16_t    trigger_amplifier() const;
   uint16_t    circulator_temperature() const;
   uint16_t    spectrum_filter_pressure() const;
   uint16_t    wg_arc_vswr() const;
   uint16_t    cabinet_interlock() const;
   uint16_t    cabinet_air_temperature() const;
   uint16_t    cabinet_airflow() const;
   uint16_t    klystron_current() const;
   uint16_t    klystron_filament_current() const;
   uint16_t    klystron_vacion_current() const;
   uint16_t    klystron_air_temperature() const;
   uint16_t    klystron_airflow() const;
   uint16_t    modulator_switch_maintenance() const;
   uint16_t    post_charge_regulator_maintenance() const;
   uint16_t    wg_pressure_humidity() const;
   uint16_t    transmitter_overvoltage() const;
   uint16_t    transmitter_overcurrent() const;
   uint16_t    focus_coil_current() const;
   uint16_t    focus_coil_airflow() const;
   uint16_t    oil_temperature() const;
   uint16_t    prf_limit() const;
   uint16_t    transmitter_oil_level() const;
   uint16_t    transmitter_battery_charging() const;
   uint16_t    high_voltage_status() const;
   uint16_t    transmitter_recycling_summary() const;
   uint16_t    transmitter_inoperable() const;
   uint16_t    transmitter_air_filter() const;
   uint16_t    zero_test_bit(unsigned i) const;
   uint16_t    one_test_bit(unsigned i) const;
   uint16_t    xmtr_spip_interface() const;
   uint16_t    transmitter_summary_status() const;
   float       transmitter_rf_power() const;
   float       horizontal_xmtr_peak_power() const;
   float       xmtr_peak_power() const;
   float       vertical_xmtr_peak_power() const;
   float       xmtr_rf_avg_power() const;
   uint32_t    xmtr_recycle_count() const;
   float       receiver_bias() const;
   float       transmit_imbalance() const;
   float       xmtr_power_meter_zero() const;
   uint16_t    ac_unit1_compressor_shut_off() const;
   uint16_t    ac_unit2_compressor_shut_off() const;
   uint16_t    generator_maintenance_required() const;
   uint16_t    generator_battery_voltage() const;
   uint16_t    generator_engine() const;
   uint16_t    generator_volt_frequency() const;
   uint16_t    power_source() const;
   uint16_t    transitional_power_source() const;
   uint16_t    generator_auto_run_off_switch() const;
   uint16_t    aircraft_hazard_lighting() const;
   uint16_t    equipment_shelter_fire_detection_system() const;
   uint16_t    equipment_shelter_fire_smoke() const;
   uint16_t    generator_shelter_fire_smoke() const;
   uint16_t    utility_voltage_frequency() const;
   uint16_t    site_security_alarm() const;
   uint16_t    security_equipment() const;
   uint16_t    security_system() const;
   uint16_t    receiver_connected_to_antenna() const;
   uint16_t    radome_hatch() const;
   uint16_t    ac_unit1_filter_dirty() const;
   uint16_t    ac_unit2_filter_dirty() const;
   float       equipment_shelter_temperature() const;
   float       outside_ambient_temperature() const;
   float       transmitter_leaving_air_temp() const;
   float       ac_unit1_discharge_air_temp() const;
   float       generator_shelter_temperature() const;
   float       radome_air_temperature() const;
   float       ac_unit2_discharge_air_temp() const;
   float       spip_15v_ps() const;
   float       spip_neg_15v_ps() const;
   uint16_t    spip_28v_ps_status() const;
   float       spip_5v_ps() const;
   uint16_t    converted_generator_fuel_level() const;
   uint16_t    elevation_pos_dead_limit() const;
   uint16_t    _150v_overvoltage() const;
   uint16_t    _150v_undervoltage() const;
   uint16_t    elevation_servo_amp_inhibit() const;
   uint16_t    elevation_servo_amp_short_circuit() const;
   uint16_t    elevation_servo_amp_overtemp() const;
   uint16_t    elevation_motor_overtemp() const;
   uint16_t    elevation_stow_pin() const;
   uint16_t    elevation_housing_5v_ps() const;
   uint16_t    elevation_neg_dead_limit() const;
   uint16_t    elevation_pos_normal_limit() const;
   uint16_t    elevation_neg_normal_limit() const;
   uint16_t    elevation_encoder_light() const;
   uint16_t    elevation_gearbox_oil() const;
   uint16_t    elevation_handwheel() const;
   uint16_t    elevation_amp_ps() const;
   uint16_t    azimuth_servo_amp_inhibit() const;
   uint16_t    azimuth_servo_amp_short_circuit() const;
   uint16_t    azimuth_servo_amp_overtemp() const;
   uint16_t    azimuth_motor_overtemp() const;
   uint16_t    azimuth_stow_pin() const;
   uint16_t    azimuth_housing_5v_ps() const;
   uint16_t    azimuth_encoder_light() const;
   uint16_t    azimuth_gearbox_oil() const;
   uint16_t    azimuth_bull_gear_oil() const;
   uint16_t    azimuth_handwheel() const;
   uint16_t    azimuth_servo_amp_ps() const;
   uint16_t    servo() const;
   uint16_t    pedestal_interlock_switch() const;
   uint16_t    coho_clock() const;
   uint16_t    rf_generator_frequency_select_oscillator() const;
   uint16_t    rf_generator_rf_stalo() const;
   uint16_t    rf_generator_phase_shifted_coho() const;
   uint16_t    _9v_receiver_ps() const;
   uint16_t    _5v_receiver_ps() const;
   uint16_t    _18v_receiver_ps() const;
   uint16_t    neg_9v_receiver_ps() const;
   uint16_t    _5v_single_channel_rdaiu_ps() const;
   float       horizontal_short_pulse_noise() const;
   float       horizontal_long_pulse_noise() const;
   float       horizontal_noise_temperature() const;
   float       vertical_short_pulse_noise() const;
   float       vertical_long_pulse_noise() const;
   float       vertical_noise_temperature() const;
   float       horizontal_linearity() const;
   float       horizontal_dynamic_range() const;
   float       horizontal_delta_dbz0() const;
   float       vertical_delta_dbz0() const;
   float       kd_peak_measured() const;
   float       short_pulse_horizontal_dbz0() const;
   float       long_pulse_horizontal_dbz0() const;
   uint16_t    velocity_processed() const;
   uint16_t    width_processed() const;
   uint16_t    velocity_rf_gen() const;
   uint16_t    width_rf_gen() const;
   float       horizontal_i0() const;
   float       vertical_i0() const;
   float       vertical_dynamic_range() const;
   float       short_pulse_vertical_dbz0() const;
   float       long_pulse_vertical_dbz0() const;
   float       horizontal_power_sense() const;
   float       vertical_power_sense() const;
   float       zdr_offset() const;
   float       clutter_suppression_delta() const;
   float       clutter_suppression_unfiltered_power() const;
   float       clutter_suppression_filtered_power() const;
   float       vertical_linearity() const;
   uint16_t    state_file_read_status() const;
   uint16_t    state_file_write_status() const;
   uint16_t    bypass_map_file_read_status() const;
   uint16_t    bypass_map_file_write_status() const;
   uint16_t    current_adaptation_file_read_status() const;
   uint16_t    current_adaptation_file_write_status() const;
   uint16_t    censor_zone_file_read_status() const;
   uint16_t    censor_zone_file_write_status() const;
   uint16_t    remote_vcp_file_read_status() const;
   uint16_t    remote_vcp_file_write_status() const;
   uint16_t    baseline_adaptation_file_read_status() const;
   uint16_t    read_status_of_prf_sets() const;
   uint16_t    clutter_filter_map_file_read_status() const;
   uint16_t    clutter_filter_map_file_write_status() const;
   uint16_t    general_disk_io_error() const;
   uint8_t     rsp_status() const;
   uint8_t     cpu1_temperature() const;
   uint8_t     cpu2_temperature() const;
   uint16_t    rsp_motherboard_power() const;
   uint16_t    spip_comm_status() const;
   uint16_t    hci_comm_status() const;
   uint16_t    signal_processor_command_status() const;
   uint16_t    ame_communication_status() const;
   uint16_t    rms_link_status() const;
   uint16_t    rpg_link_status() const;
   uint16_t    interpanel_link_status() const;
   uint32_t    performance_check_time() const;
   uint16_t    version() const;

   bool Parse(std::istream& is);

   static std::shared_ptr<PerformanceMaintenanceData>
   Create(Level2MessageHeader&& header, std::istream& is);

private:
   std::unique_ptr<PerformanceMaintenanceDataImpl> p;
};

} // namespace rda
} // namespace wsr88d
} // namespace scwx
