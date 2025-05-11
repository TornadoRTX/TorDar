#include <scwx/wsr88d/rda/performance_maintenance_data.hpp>
#include <scwx/util/logger.hpp>

#include <array>

namespace scwx
{
namespace wsr88d
{
namespace rda
{

static const std::string logPrefix_ =
   "scwx::wsr88d::rda::performance_maintenance_data";
static const auto logger_ = util::Logger::Create(logPrefix_);

class PerformanceMaintenanceDataImpl
{
public:
   explicit PerformanceMaintenanceDataImpl() = default;
   ~PerformanceMaintenanceDataImpl()         = default;

   // Communications
   uint16_t loopBackSetStatus_ {0};
   uint32_t t1OutputFrames_ {0};
   uint32_t t1InputFrames_ {0};
   uint32_t routerMemoryUsed_ {0};
   uint32_t routerMemoryFree_ {0};
   uint16_t routerMemoryUtilization_ {0};
   uint16_t routeToRpg_ {0};
   uint16_t t1PortStatus_ {0};
   uint16_t routerDedicatedEthernetPortStatus_ {0};
   uint16_t routerCommercialEthernetPortStatus_ {0};
   uint32_t csu24HrErroredSeconds_ {0};
   uint32_t csu24HrSeverelyErroredSeconds_ {0};
   uint32_t csu24HrSeverelyErroredFramingSeconds_ {0};
   uint32_t csu24HrUnavailableSeconds_ {0};
   uint32_t csu24HrControlledSlipSeconds_ {0};
   uint32_t csu24HrPathCodingViolations_ {0};
   uint32_t csu24HrLineErroredSeconds_ {0};
   uint32_t csu24HrBurstyErroredSeconds_ {0};
   uint32_t csu24HrDegradedMinutes_ {0};
   uint32_t lanSwitchCpuUtilization_ {0};
   uint16_t lanSwitchMemoryUtilization_ {0};
   uint16_t ifdrChasisTemperature_ {0};
   uint16_t ifdrFpgaTemperature_ {0};
   uint16_t ntpStatus_ {0};
   uint16_t ipcStatus_ {0};
   uint16_t commandedChannelControl_ {0};

   // AME
   uint16_t polarization_ {0};
   float    ameInternalTemperature_ {0.0f};
   float    ameReceiverModuleTemperature_ {0.0f};
   float    ameBiteCalModuleTemperature_ {0.0f};
   uint16_t amePeltierPulseWidthModulation_ {0};
   uint16_t amePeltierStatus_ {0};
   uint16_t ameADConverterStatus_ {0};
   uint16_t ameState_ {0};
   float    ame3_3VPsVoltage_ {0.0f};
   float    ame5VPsVoltage_ {0.0f};
   float    ame6_5VPsVoltage_ {0.0f};
   float    ame15VPsVoltage_ {0.0f};
   float    ame48VPsVoltage_ {0.0f};
   float    ameStaloPower_ {0.0f};
   float    peltierCurrent_ {0.0f};
   float    adcCalibrationReferenceVoltage_ {0.0f};
   uint16_t ameMode_ {0};
   uint16_t amePeltierMode_ {0};
   float    amePeltierInsideFanCurrent_ {0.0f};
   float    amePeltierOutsideFanCurrent_ {0.0f};
   float    horizontalTrLimiterVoltage_ {0.0f};
   float    verticalTrLimiterVoltage_ {0.0f};
   float    adcCalibrationOffsetVoltage_ {0.0f};
   float    adcCalibrationGainCorrection_ {0.0f};

   // RCP/SPIP Power Button Status
   uint16_t    rcpStatus_ {0};
   std::string rcpString_ {};
   uint16_t    spipPowerButtons_ {0};

   // Power
   float masterPowerAdministratorLoad_ {0.0f};
   float expansionPowerAdministratorLoad_ {0.0f};

   // Transmitter
   uint16_t                _5VdcPs_ {0};
   uint16_t                _15VdcPs_ {0};
   uint16_t                _28VdcPs_ {0};
   uint16_t                neg15VdcPs_ {0};
   uint16_t                _45VdcPs_ {0};
   uint16_t                filamentPsVoltage_ {0};
   uint16_t                vacuumPumpPsVoltage_ {0};
   uint16_t                focusCoilPsVoltage_ {0};
   uint16_t                filamentPs_ {0};
   uint16_t                klystronWarmup_ {0};
   uint16_t                transmitterAvailable_ {0};
   uint16_t                wgSwitchPosition_ {0};
   uint16_t                wgPfnTransferInterlock_ {0};
   uint16_t                maintenanceMode_ {0};
   uint16_t                maintenanceRequired_ {0};
   uint16_t                pfnSwitchPosition_ {0};
   uint16_t                modulatorOverload_ {0};
   uint16_t                modulatorInvCurrent_ {0};
   uint16_t                modulatorSwitchFail_ {0};
   uint16_t                mainPowerVoltage_ {0};
   uint16_t                chargingSystemFail_ {0};
   uint16_t                inverseDiodeCurrent_ {0};
   uint16_t                triggerAmplifier_ {0};
   uint16_t                circulatorTemperature_ {0};
   uint16_t                spectrumFilterPressure_ {0};
   uint16_t                wgArcVswr_ {0};
   uint16_t                cabinetInterlock_ {0};
   uint16_t                cabinetAirTemperature_ {0};
   uint16_t                cabinetAirflow_ {0};
   uint16_t                klystronCurrent_ {0};
   uint16_t                klystronFilamentCurrent_ {0};
   uint16_t                klystronVacionCurrent_ {0};
   uint16_t                klystronAirTemperature_ {0};
   uint16_t                klystronAirflow_ {0};
   uint16_t                modulatorSwitchMaintenance_ {0};
   uint16_t                postChargeRegulatorMaintenance_ {0};
   uint16_t                wgPressureHumidity_ {0};
   uint16_t                transmitterOvervoltage_ {0};
   uint16_t                transmitterOvercurrent_ {0};
   uint16_t                focusCoilCurrent_ {0};
   uint16_t                focusCoilAirflow_ {0};
   uint16_t                oilTemperature_ {0};
   uint16_t                prfLimit_ {0};
   uint16_t                transmitterOilLevel_ {0};
   uint16_t                transmitterBatteryCharging_ {0};
   uint16_t                highVoltageStatus_ {0};
   uint16_t                transmitterRecyclingSummary_ {0};
   uint16_t                transmitterInoperable_ {0};
   uint16_t                transmitterAirFilter_ {0};
   std::array<uint16_t, 8> zeroTestBit_ {0};
   std::array<uint16_t, 8> oneTestBit_ {0};
   uint16_t                xmtrSpipInterface_ {0};
   uint16_t                transmitterSummaryStatus_ {0};
   float                   transmitterRfPower_ {0.0f};
   float                   horizontalXmtrPeakPower_ {0.0f};
   float                   xmtrPeakPower_ {0.0f};
   float                   verticalXmtrPeakPower_ {0.0f};
   float                   xmtrRfAvgPower_ {0.0f};
   uint32_t                xmtrRecycleCount_ {0};
   float                   receiverBias_ {0.0f};
   float                   transmitImbalance_ {0.0f};
   float                   xmtrPowerMeterZero_ {0.0f};

   // Tower/Utilities
   uint16_t acUnit1CompressorShutOff_ {0};
   uint16_t acUnit2CompressorShutOff_ {0};
   uint16_t generatorMaintenanceRequired_ {0};
   uint16_t generatorBatteryVoltage_ {0};
   uint16_t generatorEngine_ {0};
   uint16_t generatorVoltFrequency_ {0};
   uint16_t powerSource_ {0};
   uint16_t transitionalPowerSource_ {0};
   uint16_t generatorAutoRunOffSwitch_ {0};
   uint16_t aircraftHazardLighting_ {0};

   // Equipment Shelter
   uint16_t equipmentShelterFireDetectionSystem_ {0};
   uint16_t equipmentShelterFireSmoke_ {0};
   uint16_t generatorShelterFireSmoke_ {0};
   uint16_t utilityVoltageFrequency_ {0};
   uint16_t siteSecurityAlarm_ {0};
   uint16_t securityEquipment_ {0};
   uint16_t securitySystem_ {0};
   uint16_t receiverConnectedToAntenna_ {0};
   uint16_t radomeHatch_ {0};
   uint16_t acUnit1FilterDirty_ {0};
   uint16_t acUnit2FilterDirty_ {0};
   float    equipmentShelterTemperature_ {0.0f};
   float    outsideAmbientTemperature_ {0.0f};
   float    transmitterLeavingAirTemp_ {0.0f};
   float    acUnit1DischargeAirTemp_ {0.0f};
   float    generatorShelterTemperature_ {0.0f};
   float    radomeAirTemperature_ {0.0f};
   float    acUnit2DischargeAirTemp_ {0.0f};
   float    spip15VPs_ {0.0f};
   float    spipNeg15VPs_ {0.0f};
   uint16_t spip28VPsStatus_ {0};
   float    spip5VPs_ {0.0f};
   uint16_t convertedGeneratorFuelLevel_ {0};

   // Antenna/Pedestal
   uint16_t elevationPosDeadLimit_ {0};
   uint16_t _150VOvervoltage_ {0};
   uint16_t _150VUndervoltage_ {0};
   uint16_t elevationServoAmpInhibit_ {0};
   uint16_t elevationServoAmpShortCircuit_ {0};
   uint16_t elevationServoAmpOvertemp_ {0};
   uint16_t elevationMotorOvertemp_ {0};
   uint16_t elevationStowPin_ {0};
   uint16_t elevationHousing5VPs_ {0};
   uint16_t elevationNegDeadLimit_ {0};
   uint16_t elevationPosNormalLimit_ {0};
   uint16_t elevationNegNormalLimit_ {0};
   uint16_t elevationEncoderLight_ {0};
   uint16_t elevationGearboxOil_ {0};
   uint16_t elevationHandwheel_ {0};
   uint16_t elevationAmpPs_ {0};
   uint16_t azimuthServoAmpInhibit_ {0};
   uint16_t azimuthServoAmpShortCircuit_ {0};
   uint16_t azimuthServoAmpOvertemp_ {0};
   uint16_t azimuthMotorOvertemp_ {0};
   uint16_t azimuthStowPin_ {0};
   uint16_t azimuthHousing5VPs_ {0};
   uint16_t azimuthEncoderLight_ {0};
   uint16_t azimuthGearboxOil_ {0};
   uint16_t azimuthBullGearOil_ {0};
   uint16_t azimuthHandwheel_ {0};
   uint16_t azimuthServoAmpPs_ {0};
   uint16_t servo_ {0};
   uint16_t pedestalInterlockSwitch_ {0};

   // RF Generator/Receiver
   uint16_t cohoClock_ {0};
   uint16_t rfGeneratorFrequencySelectOscillator_ {0};
   uint16_t rfGeneratorRfStalo_ {0};
   uint16_t rfGeneratorPhaseShiftedCoho_ {0};
   uint16_t _9VReceiverPs_ {0};
   uint16_t _5VReceiverPs_ {0};
   uint16_t _18VReceiverPs_ {0};
   uint16_t neg9VReceiverPs_ {0};
   uint16_t _5VSingleChannelRdaiuPs_ {0};
   float    horizontalShortPulseNoise_ {0.0f};
   float    horizontalLongPulseNoise_ {0.0f};
   float    horizontalNoiseTemperature_ {0.0f};
   float    verticalShortPulseNoise_ {0.0f};
   float    verticalLongPulseNoise_ {0.0f};
   float    verticalNoiseTemperature_ {0.0f};

   // Calibration
   float    horizontalLinearity_ {0.0f};
   float    horizontalDynamicRange_ {0.0f};
   float    horizontalDeltaDbz0_ {0.0f};
   float    verticalDeltaDbz0_ {0.0f};
   float    kdPeakMeasured_ {0.0f};
   float    shortPulseHorizontalDbz0_ {0.0f};
   float    longPulseHorizontalDbz0_ {0.0f};
   uint16_t velocityProcessed_ {0};
   uint16_t widthProcessed_ {0};
   uint16_t velocityRfGen_ {0};
   uint16_t widthRfGen_ {0};
   float    horizontalI0_ {0.0f};
   float    verticalI0_ {0.0f};
   float    verticalDynamicRange_ {0.0f};
   float    shortPulseVerticalDbz0_ {0.0f};
   float    longPulseVerticalDbz0_ {0.0f};
   float    horizontalPowerSense_ {0.0f};
   float    verticalPowerSense_ {0.0f};
   float    zdrOffset_ {0.0f};
   float    clutterSuppressionDelta_ {0.0f};
   float    clutterSuppressionUnfilteredPower_ {0.0f};
   float    clutterSuppressionFilteredPower_ {0.0f};
   float    verticalLinearity_ {0.0f};

   // File Status
   uint16_t stateFileReadStatus_ {0};
   uint16_t stateFileWriteStatus_ {0};
   uint16_t bypassMapFileReadStatus_ {0};
   uint16_t bypassMapFileWriteStatus_ {0};
   uint16_t currentAdaptationFileReadStatus_ {0};
   uint16_t currentAdaptationFileWriteStatus_ {0};
   uint16_t censorZoneFileReadStatus_ {0};
   uint16_t censorZoneFileWriteStatus_ {0};
   uint16_t remoteVcpFileReadStatus_ {0};
   uint16_t remoteVcpFileWriteStatus_ {0};
   uint16_t baselineAdaptationFileReadStatus_ {0};
   uint16_t readStatusOfPrfSets_ {0};
   uint16_t clutterFilterMapFileReadStatus_ {0};
   uint16_t clutterFilterMapFileWriteStatus_ {0};
   uint16_t generalDiskIoError_ {0};
   uint8_t  rspStatus_ {0};
   uint8_t  cpu1Temperature_ {0};
   uint8_t  cpu2Temperature_ {0};
   uint16_t rspMotherboardPower_ {0};

   // Device Status
   uint16_t spipCommStatus_ {0};
   uint16_t hciCommStatus_ {0};
   uint16_t signalProcessorCommandStatus_ {0};
   uint16_t ameCommunicationStatus_ {0};
   uint16_t rmsLinkStatus_ {0};
   uint16_t rpgLinkStatus_ {0};
   uint16_t interpanelLinkStatus_ {0};
   uint32_t performanceCheckTime_ {0};
   uint16_t version_ {0};
};

PerformanceMaintenanceData::PerformanceMaintenanceData() :
    Level2Message(), p(std::make_unique<PerformanceMaintenanceDataImpl>())
{
}
PerformanceMaintenanceData::~PerformanceMaintenanceData() = default;

PerformanceMaintenanceData::PerformanceMaintenanceData(
   PerformanceMaintenanceData&&) noexcept = default;
PerformanceMaintenanceData& PerformanceMaintenanceData::operator=(
   PerformanceMaintenanceData&&) noexcept = default;

uint16_t PerformanceMaintenanceData::loop_back_set_status() const
{
   return p->loopBackSetStatus_;
}

uint32_t PerformanceMaintenanceData::t1_output_frames() const
{
   return p->t1OutputFrames_;
}

uint32_t PerformanceMaintenanceData::t1_input_frames() const
{
   return p->t1InputFrames_;
}

uint32_t PerformanceMaintenanceData::router_memory_used() const
{
   return p->routerMemoryUsed_;
}

uint32_t PerformanceMaintenanceData::router_memory_free() const
{
   return p->routerMemoryFree_;
}

uint16_t PerformanceMaintenanceData::router_memory_utilization() const
{
   return p->routerMemoryUtilization_;
}

uint16_t PerformanceMaintenanceData::route_to_rpg() const
{
   return p->routeToRpg_;
}

uint16_t PerformanceMaintenanceData::t1_port_status() const
{
   return p->t1PortStatus_;
}

uint16_t
PerformanceMaintenanceData::router_dedicated_ethernet_port_status() const
{
   return p->routerDedicatedEthernetPortStatus_;
}

uint16_t
PerformanceMaintenanceData::router_commercial_ethernet_port_status() const
{
   return p->routerCommercialEthernetPortStatus_;
}

uint32_t PerformanceMaintenanceData::csu_24hr_errored_seconds() const
{
   return p->csu24HrErroredSeconds_;
}

uint32_t PerformanceMaintenanceData::csu_24hr_severely_errored_seconds() const
{
   return p->csu24HrSeverelyErroredSeconds_;
}

uint32_t
PerformanceMaintenanceData::csu_24hr_severely_errored_framing_seconds() const
{
   return p->csu24HrSeverelyErroredFramingSeconds_;
}

uint32_t PerformanceMaintenanceData::csu_24hr_unavailable_seconds() const
{
   return p->csu24HrUnavailableSeconds_;
}

uint32_t PerformanceMaintenanceData::csu_24hr_controlled_slip_seconds() const
{
   return p->csu24HrControlledSlipSeconds_;
}

uint32_t PerformanceMaintenanceData::csu_24hr_path_coding_violations() const
{
   return p->csu24HrPathCodingViolations_;
}

uint32_t PerformanceMaintenanceData::csu_24hr_line_errored_seconds() const
{
   return p->csu24HrLineErroredSeconds_;
}

uint32_t PerformanceMaintenanceData::csu_24hr_bursty_errored_seconds() const
{
   return p->csu24HrBurstyErroredSeconds_;
}

uint32_t PerformanceMaintenanceData::csu_24hr_degraded_minutes() const
{
   return p->csu24HrDegradedMinutes_;
}

uint32_t PerformanceMaintenanceData::lan_switch_cpu_utilization() const
{
   return p->lanSwitchCpuUtilization_;
}

uint16_t PerformanceMaintenanceData::lan_switch_memory_utilization() const
{
   return p->lanSwitchMemoryUtilization_;
}

uint16_t PerformanceMaintenanceData::ifdr_chasis_temperature() const
{
   return p->ifdrChasisTemperature_;
}

uint16_t PerformanceMaintenanceData::ifdr_fpga_temperature() const
{
   return p->ifdrFpgaTemperature_;
}

uint16_t PerformanceMaintenanceData::ntp_status() const
{
   return p->ntpStatus_;
}

uint16_t PerformanceMaintenanceData::ipc_status() const
{
   return p->ipcStatus_;
}

uint16_t PerformanceMaintenanceData::commanded_channel_control() const
{
   return p->commandedChannelControl_;
}

uint16_t PerformanceMaintenanceData::polarization() const
{
   return p->polarization_;
}

float PerformanceMaintenanceData::ame_internal_temperature() const
{
   return p->ameInternalTemperature_;
}

float PerformanceMaintenanceData::ame_receiver_module_temperature() const
{
   return p->ameReceiverModuleTemperature_;
}

float PerformanceMaintenanceData::ame_bite_cal_module_temperature() const
{
   return p->ameBiteCalModuleTemperature_;
}

uint16_t PerformanceMaintenanceData::ame_peltier_pulse_width_modulation() const
{
   return p->amePeltierPulseWidthModulation_;
}

uint16_t PerformanceMaintenanceData::ame_peltier_status() const
{
   return p->amePeltierStatus_;
}

uint16_t PerformanceMaintenanceData::ame_a_d_converter_status() const
{
   return p->ameADConverterStatus_;
}

uint16_t PerformanceMaintenanceData::ame_state() const
{
   return p->ameState_;
}

float PerformanceMaintenanceData::ame_3_3v_ps_voltage() const
{
   return p->ame3_3VPsVoltage_;
}

float PerformanceMaintenanceData::ame_5v_ps_voltage() const
{
   return p->ame5VPsVoltage_;
}

float PerformanceMaintenanceData::ame_6_5v_ps_voltage() const
{
   return p->ame6_5VPsVoltage_;
}

float PerformanceMaintenanceData::ame_15v_ps_voltage() const
{
   return p->ame15VPsVoltage_;
}

float PerformanceMaintenanceData::ame_48v_ps_voltage() const
{
   return p->ame48VPsVoltage_;
}

float PerformanceMaintenanceData::ame_stalo_power() const
{
   return p->ameStaloPower_;
}

float PerformanceMaintenanceData::peltier_current() const
{
   return p->peltierCurrent_;
}

float PerformanceMaintenanceData::adc_calibration_reference_voltage() const
{
   return p->adcCalibrationReferenceVoltage_;
}

uint16_t PerformanceMaintenanceData::ame_mode() const
{
   return p->ameMode_;
}

uint16_t PerformanceMaintenanceData::ame_peltier_mode() const
{
   return p->amePeltierMode_;
}

float PerformanceMaintenanceData::ame_peltier_inside_fan_current() const
{
   return p->amePeltierInsideFanCurrent_;
}

float PerformanceMaintenanceData::ame_peltier_outside_fan_current() const
{
   return p->amePeltierOutsideFanCurrent_;
}

float PerformanceMaintenanceData::horizontal_tr_limiter_voltage() const
{
   return p->horizontalTrLimiterVoltage_;
}

float PerformanceMaintenanceData::vertical_tr_limiter_voltage() const
{
   return p->verticalTrLimiterVoltage_;
}

float PerformanceMaintenanceData::adc_calibration_offset_voltage() const
{
   return p->adcCalibrationOffsetVoltage_;
}

float PerformanceMaintenanceData::adc_calibration_gain_correction() const
{
   return p->adcCalibrationGainCorrection_;
}

uint16_t PerformanceMaintenanceData::rcp_status() const
{
   return p->rcpStatus_;
}

std::string PerformanceMaintenanceData::rcp_string() const
{
   return p->rcpString_;
}

uint16_t PerformanceMaintenanceData::spip_power_buttons() const
{
   return p->spipPowerButtons_;
}

float PerformanceMaintenanceData::master_power_administrator_load() const
{
   return p->masterPowerAdministratorLoad_;
}

float PerformanceMaintenanceData::expansion_power_administrator_load() const
{
   return p->expansionPowerAdministratorLoad_;
}

uint16_t PerformanceMaintenanceData::_5vdc_ps() const
{
   return p->_5VdcPs_;
}

uint16_t PerformanceMaintenanceData::_15vdc_ps() const
{
   return p->_15VdcPs_;
}

uint16_t PerformanceMaintenanceData::_28vdc_ps() const
{
   return p->_28VdcPs_;
}

uint16_t PerformanceMaintenanceData::neg_15vdc_ps() const
{
   return p->neg15VdcPs_;
}

uint16_t PerformanceMaintenanceData::_45vdc_ps() const
{
   return p->_45VdcPs_;
}

uint16_t PerformanceMaintenanceData::filament_ps_voltage() const
{
   return p->filamentPsVoltage_;
}

uint16_t PerformanceMaintenanceData::vacuum_pump_ps_voltage() const
{
   return p->vacuumPumpPsVoltage_;
}

uint16_t PerformanceMaintenanceData::focus_coil_ps_voltage() const
{
   return p->focusCoilPsVoltage_;
}

uint16_t PerformanceMaintenanceData::filament_ps() const
{
   return p->filamentPs_;
}

uint16_t PerformanceMaintenanceData::klystron_warmup() const
{
   return p->klystronWarmup_;
}

uint16_t PerformanceMaintenanceData::transmitter_available() const
{
   return p->transmitterAvailable_;
}

uint16_t PerformanceMaintenanceData::wg_switch_position() const
{
   return p->wgSwitchPosition_;
}

uint16_t PerformanceMaintenanceData::wg_pfn_transfer_interlock() const
{
   return p->wgPfnTransferInterlock_;
}

uint16_t PerformanceMaintenanceData::maintenance_mode() const
{
   return p->maintenanceMode_;
}

uint16_t PerformanceMaintenanceData::maintenance_required() const
{
   return p->maintenanceRequired_;
}

uint16_t PerformanceMaintenanceData::pfn_switch_position() const
{
   return p->pfnSwitchPosition_;
}

uint16_t PerformanceMaintenanceData::modulator_overload() const
{
   return p->modulatorOverload_;
}

uint16_t PerformanceMaintenanceData::modulator_inv_current() const
{
   return p->modulatorInvCurrent_;
}

uint16_t PerformanceMaintenanceData::modulator_switch_fail() const
{
   return p->modulatorSwitchFail_;
}

uint16_t PerformanceMaintenanceData::main_power_voltage() const
{
   return p->mainPowerVoltage_;
}

uint16_t PerformanceMaintenanceData::charging_system_fail() const
{
   return p->chargingSystemFail_;
}

uint16_t PerformanceMaintenanceData::inverse_diode_current() const
{
   return p->inverseDiodeCurrent_;
}

uint16_t PerformanceMaintenanceData::trigger_amplifier() const
{
   return p->triggerAmplifier_;
}

uint16_t PerformanceMaintenanceData::circulator_temperature() const
{
   return p->circulatorTemperature_;
}

uint16_t PerformanceMaintenanceData::spectrum_filter_pressure() const
{
   return p->spectrumFilterPressure_;
}

uint16_t PerformanceMaintenanceData::wg_arc_vswr() const
{
   return p->wgArcVswr_;
}

uint16_t PerformanceMaintenanceData::cabinet_interlock() const
{
   return p->cabinetInterlock_;
}

uint16_t PerformanceMaintenanceData::cabinet_air_temperature() const
{
   return p->cabinetAirTemperature_;
}

uint16_t PerformanceMaintenanceData::cabinet_airflow() const
{
   return p->cabinetAirflow_;
}

uint16_t PerformanceMaintenanceData::klystron_current() const
{
   return p->klystronCurrent_;
}

uint16_t PerformanceMaintenanceData::klystron_filament_current() const
{
   return p->klystronFilamentCurrent_;
}

uint16_t PerformanceMaintenanceData::klystron_vacion_current() const
{
   return p->klystronVacionCurrent_;
}

uint16_t PerformanceMaintenanceData::klystron_air_temperature() const
{
   return p->klystronAirTemperature_;
}

uint16_t PerformanceMaintenanceData::klystron_airflow() const
{
   return p->klystronAirflow_;
}

uint16_t PerformanceMaintenanceData::modulator_switch_maintenance() const
{
   return p->modulatorSwitchMaintenance_;
}

uint16_t PerformanceMaintenanceData::post_charge_regulator_maintenance() const
{
   return p->postChargeRegulatorMaintenance_;
}

uint16_t PerformanceMaintenanceData::wg_pressure_humidity() const
{
   return p->wgPressureHumidity_;
}

uint16_t PerformanceMaintenanceData::transmitter_overvoltage() const
{
   return p->transmitterOvervoltage_;
}

uint16_t PerformanceMaintenanceData::transmitter_overcurrent() const
{
   return p->transmitterOvercurrent_;
}

uint16_t PerformanceMaintenanceData::focus_coil_current() const
{
   return p->focusCoilCurrent_;
}

uint16_t PerformanceMaintenanceData::focus_coil_airflow() const
{
   return p->focusCoilAirflow_;
}

uint16_t PerformanceMaintenanceData::oil_temperature() const
{
   return p->oilTemperature_;
}

uint16_t PerformanceMaintenanceData::prf_limit() const
{
   return p->prfLimit_;
}

uint16_t PerformanceMaintenanceData::transmitter_oil_level() const
{
   return p->transmitterOilLevel_;
}

uint16_t PerformanceMaintenanceData::transmitter_battery_charging() const
{
   return p->transmitterBatteryCharging_;
}

uint16_t PerformanceMaintenanceData::high_voltage_status() const
{
   return p->highVoltageStatus_;
}

uint16_t PerformanceMaintenanceData::transmitter_recycling_summary() const
{
   return p->transmitterRecyclingSummary_;
}

uint16_t PerformanceMaintenanceData::transmitter_inoperable() const
{
   return p->transmitterInoperable_;
}

uint16_t PerformanceMaintenanceData::transmitter_air_filter() const
{
   return p->transmitterAirFilter_;
}

uint16_t PerformanceMaintenanceData::zero_test_bit(unsigned i) const
{
   return p->zeroTestBit_[i];
}

uint16_t PerformanceMaintenanceData::one_test_bit(unsigned i) const
{
   return p->oneTestBit_[i];
}

uint16_t PerformanceMaintenanceData::xmtr_spip_interface() const
{
   return p->xmtrSpipInterface_;
}

uint16_t PerformanceMaintenanceData::transmitter_summary_status() const
{
   return p->transmitterSummaryStatus_;
}

float PerformanceMaintenanceData::transmitter_rf_power() const
{
   return p->transmitterRfPower_;
}

float PerformanceMaintenanceData::horizontal_xmtr_peak_power() const
{
   return p->horizontalXmtrPeakPower_;
}

float PerformanceMaintenanceData::xmtr_peak_power() const
{
   return p->xmtrPeakPower_;
}

float PerformanceMaintenanceData::vertical_xmtr_peak_power() const
{
   return p->verticalXmtrPeakPower_;
}

float PerformanceMaintenanceData::xmtr_rf_avg_power() const
{
   return p->xmtrRfAvgPower_;
}

uint32_t PerformanceMaintenanceData::xmtr_recycle_count() const
{
   return p->xmtrRecycleCount_;
}

float PerformanceMaintenanceData::receiver_bias() const
{
   return p->receiverBias_;
}

float PerformanceMaintenanceData::transmit_imbalance() const
{
   return p->transmitImbalance_;
}

float PerformanceMaintenanceData::xmtr_power_meter_zero() const
{
   return p->xmtrPowerMeterZero_;
}

uint16_t PerformanceMaintenanceData::ac_unit1_compressor_shut_off() const
{
   return p->acUnit1CompressorShutOff_;
}

uint16_t PerformanceMaintenanceData::ac_unit2_compressor_shut_off() const
{
   return p->acUnit2CompressorShutOff_;
}

uint16_t PerformanceMaintenanceData::generator_maintenance_required() const
{
   return p->generatorMaintenanceRequired_;
}

uint16_t PerformanceMaintenanceData::generator_battery_voltage() const
{
   return p->generatorBatteryVoltage_;
}

uint16_t PerformanceMaintenanceData::generator_engine() const
{
   return p->generatorEngine_;
}

uint16_t PerformanceMaintenanceData::generator_volt_frequency() const
{
   return p->generatorVoltFrequency_;
}

uint16_t PerformanceMaintenanceData::power_source() const
{
   return p->powerSource_;
}

uint16_t PerformanceMaintenanceData::transitional_power_source() const
{
   return p->transitionalPowerSource_;
}

uint16_t PerformanceMaintenanceData::generator_auto_run_off_switch() const
{
   return p->generatorAutoRunOffSwitch_;
}

uint16_t PerformanceMaintenanceData::aircraft_hazard_lighting() const
{
   return p->aircraftHazardLighting_;
}

uint16_t
PerformanceMaintenanceData::equipment_shelter_fire_detection_system() const
{
   return p->equipmentShelterFireDetectionSystem_;
}

uint16_t PerformanceMaintenanceData::equipment_shelter_fire_smoke() const
{
   return p->equipmentShelterFireSmoke_;
}

uint16_t PerformanceMaintenanceData::generator_shelter_fire_smoke() const
{
   return p->generatorShelterFireSmoke_;
}

uint16_t PerformanceMaintenanceData::utility_voltage_frequency() const
{
   return p->utilityVoltageFrequency_;
}

uint16_t PerformanceMaintenanceData::site_security_alarm() const
{
   return p->siteSecurityAlarm_;
}

uint16_t PerformanceMaintenanceData::security_equipment() const
{
   return p->securityEquipment_;
}

uint16_t PerformanceMaintenanceData::security_system() const
{
   return p->securitySystem_;
}

uint16_t PerformanceMaintenanceData::receiver_connected_to_antenna() const
{
   return p->receiverConnectedToAntenna_;
}

uint16_t PerformanceMaintenanceData::radome_hatch() const
{
   return p->radomeHatch_;
}

uint16_t PerformanceMaintenanceData::ac_unit1_filter_dirty() const
{
   return p->acUnit1FilterDirty_;
}

uint16_t PerformanceMaintenanceData::ac_unit2_filter_dirty() const
{
   return p->acUnit2FilterDirty_;
}

float PerformanceMaintenanceData::equipment_shelter_temperature() const
{
   return p->equipmentShelterTemperature_;
}

float PerformanceMaintenanceData::outside_ambient_temperature() const
{
   return p->outsideAmbientTemperature_;
}

float PerformanceMaintenanceData::transmitter_leaving_air_temp() const
{
   return p->transmitterLeavingAirTemp_;
}

float PerformanceMaintenanceData::ac_unit1_discharge_air_temp() const
{
   return p->acUnit1DischargeAirTemp_;
}

float PerformanceMaintenanceData::generator_shelter_temperature() const
{
   return p->generatorShelterTemperature_;
}

float PerformanceMaintenanceData::radome_air_temperature() const
{
   return p->radomeAirTemperature_;
}

float PerformanceMaintenanceData::ac_unit2_discharge_air_temp() const
{
   return p->acUnit2DischargeAirTemp_;
}

float PerformanceMaintenanceData::spip_15v_ps() const
{
   return p->spip15VPs_;
}

float PerformanceMaintenanceData::spip_neg_15v_ps() const
{
   return p->spipNeg15VPs_;
}

uint16_t PerformanceMaintenanceData::spip_28v_ps_status() const
{
   return p->spip28VPsStatus_;
}

float PerformanceMaintenanceData::spip_5v_ps() const
{
   return p->spip5VPs_;
}

uint16_t PerformanceMaintenanceData::converted_generator_fuel_level() const
{
   return p->convertedGeneratorFuelLevel_;
}

uint16_t PerformanceMaintenanceData::elevation_pos_dead_limit() const
{
   return p->elevationPosDeadLimit_;
}

uint16_t PerformanceMaintenanceData::_150v_overvoltage() const
{
   return p->_150VOvervoltage_;
}

uint16_t PerformanceMaintenanceData::_150v_undervoltage() const
{
   return p->_150VUndervoltage_;
}

uint16_t PerformanceMaintenanceData::elevation_servo_amp_inhibit() const
{
   return p->elevationServoAmpInhibit_;
}

uint16_t PerformanceMaintenanceData::elevation_servo_amp_short_circuit() const
{
   return p->elevationServoAmpShortCircuit_;
}

uint16_t PerformanceMaintenanceData::elevation_servo_amp_overtemp() const
{
   return p->elevationServoAmpOvertemp_;
}

uint16_t PerformanceMaintenanceData::elevation_motor_overtemp() const
{
   return p->elevationMotorOvertemp_;
}

uint16_t PerformanceMaintenanceData::elevation_stow_pin() const
{
   return p->elevationStowPin_;
}

uint16_t PerformanceMaintenanceData::elevation_housing_5v_ps() const
{
   return p->elevationHousing5VPs_;
}

uint16_t PerformanceMaintenanceData::elevation_neg_dead_limit() const
{
   return p->elevationNegDeadLimit_;
}

uint16_t PerformanceMaintenanceData::elevation_pos_normal_limit() const
{
   return p->elevationPosNormalLimit_;
}

uint16_t PerformanceMaintenanceData::elevation_neg_normal_limit() const
{
   return p->elevationNegNormalLimit_;
}

uint16_t PerformanceMaintenanceData::elevation_encoder_light() const
{
   return p->elevationEncoderLight_;
}

uint16_t PerformanceMaintenanceData::elevation_gearbox_oil() const
{
   return p->elevationGearboxOil_;
}

uint16_t PerformanceMaintenanceData::elevation_handwheel() const
{
   return p->elevationHandwheel_;
}

uint16_t PerformanceMaintenanceData::elevation_amp_ps() const
{
   return p->elevationAmpPs_;
}

uint16_t PerformanceMaintenanceData::azimuth_servo_amp_inhibit() const
{
   return p->azimuthServoAmpInhibit_;
}

uint16_t PerformanceMaintenanceData::azimuth_servo_amp_short_circuit() const
{
   return p->azimuthServoAmpShortCircuit_;
}

uint16_t PerformanceMaintenanceData::azimuth_servo_amp_overtemp() const
{
   return p->azimuthServoAmpOvertemp_;
}

uint16_t PerformanceMaintenanceData::azimuth_motor_overtemp() const
{
   return p->azimuthMotorOvertemp_;
}

uint16_t PerformanceMaintenanceData::azimuth_stow_pin() const
{
   return p->azimuthStowPin_;
}

uint16_t PerformanceMaintenanceData::azimuth_housing_5v_ps() const
{
   return p->azimuthHousing5VPs_;
}

uint16_t PerformanceMaintenanceData::azimuth_encoder_light() const
{
   return p->azimuthEncoderLight_;
}

uint16_t PerformanceMaintenanceData::azimuth_gearbox_oil() const
{
   return p->azimuthGearboxOil_;
}

uint16_t PerformanceMaintenanceData::azimuth_bull_gear_oil() const
{
   return p->azimuthBullGearOil_;
}

uint16_t PerformanceMaintenanceData::azimuth_handwheel() const
{
   return p->azimuthHandwheel_;
}

uint16_t PerformanceMaintenanceData::azimuth_servo_amp_ps() const
{
   return p->azimuthServoAmpPs_;
}

uint16_t PerformanceMaintenanceData::servo() const
{
   return p->servo_;
}

uint16_t PerformanceMaintenanceData::pedestal_interlock_switch() const
{
   return p->pedestalInterlockSwitch_;
}

uint16_t PerformanceMaintenanceData::coho_clock() const
{
   return p->cohoClock_;
}

uint16_t
PerformanceMaintenanceData::rf_generator_frequency_select_oscillator() const
{
   return p->rfGeneratorFrequencySelectOscillator_;
}

uint16_t PerformanceMaintenanceData::rf_generator_rf_stalo() const
{
   return p->rfGeneratorRfStalo_;
}

uint16_t PerformanceMaintenanceData::rf_generator_phase_shifted_coho() const
{
   return p->rfGeneratorPhaseShiftedCoho_;
}

uint16_t PerformanceMaintenanceData::_9v_receiver_ps() const
{
   return p->_9VReceiverPs_;
}

uint16_t PerformanceMaintenanceData::_5v_receiver_ps() const
{
   return p->_5VReceiverPs_;
}

uint16_t PerformanceMaintenanceData::_18v_receiver_ps() const
{
   return p->_18VReceiverPs_;
}

uint16_t PerformanceMaintenanceData::neg_9v_receiver_ps() const
{
   return p->neg9VReceiverPs_;
}

uint16_t PerformanceMaintenanceData::_5v_single_channel_rdaiu_ps() const
{
   return p->_5VSingleChannelRdaiuPs_;
}

float PerformanceMaintenanceData::horizontal_short_pulse_noise() const
{
   return p->horizontalShortPulseNoise_;
}

float PerformanceMaintenanceData::horizontal_long_pulse_noise() const
{
   return p->horizontalLongPulseNoise_;
}

float PerformanceMaintenanceData::horizontal_noise_temperature() const
{
   return p->horizontalNoiseTemperature_;
}

float PerformanceMaintenanceData::vertical_short_pulse_noise() const
{
   return p->verticalShortPulseNoise_;
}

float PerformanceMaintenanceData::vertical_long_pulse_noise() const
{
   return p->verticalLongPulseNoise_;
}

float PerformanceMaintenanceData::vertical_noise_temperature() const
{
   return p->verticalNoiseTemperature_;
}

float PerformanceMaintenanceData::horizontal_linearity() const
{
   return p->horizontalLinearity_;
}

float PerformanceMaintenanceData::horizontal_dynamic_range() const
{
   return p->horizontalDynamicRange_;
}

float PerformanceMaintenanceData::horizontal_delta_dbz0() const
{
   return p->horizontalDeltaDbz0_;
}

float PerformanceMaintenanceData::vertical_delta_dbz0() const
{
   return p->verticalDeltaDbz0_;
}

float PerformanceMaintenanceData::kd_peak_measured() const
{
   return p->kdPeakMeasured_;
}

float PerformanceMaintenanceData::short_pulse_horizontal_dbz0() const
{
   return p->shortPulseHorizontalDbz0_;
}

float PerformanceMaintenanceData::long_pulse_horizontal_dbz0() const
{
   return p->longPulseHorizontalDbz0_;
}

uint16_t PerformanceMaintenanceData::velocity_processed() const
{
   return p->velocityProcessed_;
}

uint16_t PerformanceMaintenanceData::width_processed() const
{
   return p->widthProcessed_;
}

uint16_t PerformanceMaintenanceData::velocity_rf_gen() const
{
   return p->velocityRfGen_;
}

uint16_t PerformanceMaintenanceData::width_rf_gen() const
{
   return p->widthRfGen_;
}

float PerformanceMaintenanceData::horizontal_i0() const
{
   return p->horizontalI0_;
}

float PerformanceMaintenanceData::vertical_i0() const
{
   return p->verticalI0_;
}

float PerformanceMaintenanceData::vertical_dynamic_range() const
{
   return p->verticalDynamicRange_;
}

float PerformanceMaintenanceData::short_pulse_vertical_dbz0() const
{
   return p->shortPulseVerticalDbz0_;
}

float PerformanceMaintenanceData::long_pulse_vertical_dbz0() const
{
   return p->longPulseVerticalDbz0_;
}

float PerformanceMaintenanceData::horizontal_power_sense() const
{
   return p->horizontalPowerSense_;
}

float PerformanceMaintenanceData::vertical_power_sense() const
{
   return p->verticalPowerSense_;
}

float PerformanceMaintenanceData::zdr_offset() const
{
   return p->zdrOffset_;
}

float PerformanceMaintenanceData::clutter_suppression_delta() const
{
   return p->clutterSuppressionDelta_;
}

float PerformanceMaintenanceData::clutter_suppression_unfiltered_power() const
{
   return p->clutterSuppressionUnfilteredPower_;
}

float PerformanceMaintenanceData::clutter_suppression_filtered_power() const
{
   return p->clutterSuppressionFilteredPower_;
}

float PerformanceMaintenanceData::vertical_linearity() const
{
   return p->verticalLinearity_;
}

uint16_t PerformanceMaintenanceData::state_file_read_status() const
{
   return p->stateFileReadStatus_;
}

uint16_t PerformanceMaintenanceData::state_file_write_status() const
{
   return p->stateFileWriteStatus_;
}

uint16_t PerformanceMaintenanceData::bypass_map_file_read_status() const
{
   return p->bypassMapFileReadStatus_;
}

uint16_t PerformanceMaintenanceData::bypass_map_file_write_status() const
{
   return p->bypassMapFileWriteStatus_;
}

uint16_t PerformanceMaintenanceData::current_adaptation_file_read_status() const
{
   return p->currentAdaptationFileReadStatus_;
}

uint16_t
PerformanceMaintenanceData::current_adaptation_file_write_status() const
{
   return p->currentAdaptationFileWriteStatus_;
}

uint16_t PerformanceMaintenanceData::censor_zone_file_read_status() const
{
   return p->censorZoneFileReadStatus_;
}

uint16_t PerformanceMaintenanceData::censor_zone_file_write_status() const
{
   return p->censorZoneFileWriteStatus_;
}

uint16_t PerformanceMaintenanceData::remote_vcp_file_read_status() const
{
   return p->remoteVcpFileReadStatus_;
}

uint16_t PerformanceMaintenanceData::remote_vcp_file_write_status() const
{
   return p->remoteVcpFileWriteStatus_;
}

uint16_t
PerformanceMaintenanceData::baseline_adaptation_file_read_status() const
{
   return p->baselineAdaptationFileReadStatus_;
}

uint16_t PerformanceMaintenanceData::read_status_of_prf_sets() const
{
   return p->readStatusOfPrfSets_;
}

uint16_t PerformanceMaintenanceData::clutter_filter_map_file_read_status() const
{
   return p->clutterFilterMapFileReadStatus_;
}

uint16_t
PerformanceMaintenanceData::clutter_filter_map_file_write_status() const
{
   return p->clutterFilterMapFileWriteStatus_;
}

uint16_t PerformanceMaintenanceData::general_disk_io_error() const
{
   return p->generalDiskIoError_;
}

uint8_t PerformanceMaintenanceData::rsp_status() const
{
   return p->rspStatus_;
}

uint8_t PerformanceMaintenanceData::cpu1_temperature() const
{
   return p->cpu1Temperature_;
}

uint8_t PerformanceMaintenanceData::cpu2_temperature() const
{
   return p->cpu2Temperature_;
}

uint16_t PerformanceMaintenanceData::rsp_motherboard_power() const
{
   return p->rspMotherboardPower_;
}

uint16_t PerformanceMaintenanceData::spip_comm_status() const
{
   return p->spipCommStatus_;
}

uint16_t PerformanceMaintenanceData::hci_comm_status() const
{
   return p->hciCommStatus_;
}

uint16_t PerformanceMaintenanceData::signal_processor_command_status() const
{
   return p->signalProcessorCommandStatus_;
}

uint16_t PerformanceMaintenanceData::ame_communication_status() const
{
   return p->ameCommunicationStatus_;
}

uint16_t PerformanceMaintenanceData::rms_link_status() const
{
   return p->rmsLinkStatus_;
}

uint16_t PerformanceMaintenanceData::rpg_link_status() const
{
   return p->rpgLinkStatus_;
}

uint16_t PerformanceMaintenanceData::interpanel_link_status() const
{
   return p->interpanelLinkStatus_;
}

uint32_t PerformanceMaintenanceData::performance_check_time() const
{
   return p->performanceCheckTime_;
}

uint16_t PerformanceMaintenanceData::version() const
{
   return p->version_;
}

bool PerformanceMaintenanceData::Parse(std::istream& is)
{
   logger_->trace("Parsing Performance/Maintenance Data (Message Type 3)");

   bool   messageValid = true;
   size_t bytesRead    = 0;

   p->rcpString_.resize(16);

   // Communications
   is.seekg(2, std::ios_base::cur);                                   // 1
   is.read(reinterpret_cast<char*>(&p->loopBackSetStatus_), 2);       // 2
   is.read(reinterpret_cast<char*>(&p->t1OutputFrames_), 4);          // 3-4
   is.read(reinterpret_cast<char*>(&p->t1InputFrames_), 4);           // 5-6
   is.read(reinterpret_cast<char*>(&p->routerMemoryUsed_), 4);        // 7-8
   is.read(reinterpret_cast<char*>(&p->routerMemoryFree_), 4);        // 9-10
   is.read(reinterpret_cast<char*>(&p->routerMemoryUtilization_), 2); // 11
   is.read(reinterpret_cast<char*>(&p->routeToRpg_), 2);              // 12
   is.read(reinterpret_cast<char*>(&p->t1PortStatus_), 2);            // 13
   is.read(reinterpret_cast<char*>(&p->routerDedicatedEthernetPortStatus_),
           2); // 14
   is.read(reinterpret_cast<char*>(&p->routerCommercialEthernetPortStatus_),
           2);                                                      // 15
   is.seekg(10, std::ios_base::cur);                                // 16-20
   is.read(reinterpret_cast<char*>(&p->csu24HrErroredSeconds_), 4); // 21-22
   is.read(reinterpret_cast<char*>(&p->csu24HrSeverelyErroredSeconds_),
           4); // 23-24
   is.read(reinterpret_cast<char*>(&p->csu24HrSeverelyErroredFramingSeconds_),
           4);                                                          // 25-26
   is.read(reinterpret_cast<char*>(&p->csu24HrUnavailableSeconds_), 4); // 27-28
   is.read(reinterpret_cast<char*>(&p->csu24HrControlledSlipSeconds_),
           4); // 29-30
   is.read(reinterpret_cast<char*>(&p->csu24HrPathCodingViolations_),
           4);                                                          // 31-32
   is.read(reinterpret_cast<char*>(&p->csu24HrLineErroredSeconds_), 4); // 33-34
   is.read(reinterpret_cast<char*>(&p->csu24HrBurstyErroredSeconds_),
           4);                                                        // 35-36
   is.read(reinterpret_cast<char*>(&p->csu24HrDegradedMinutes_), 4);  // 37-38
   is.seekg(4, std::ios_base::cur);                                   // 39-40
   is.read(reinterpret_cast<char*>(&p->lanSwitchCpuUtilization_), 4); // 41-42
   is.read(reinterpret_cast<char*>(&p->lanSwitchMemoryUtilization_), 2); // 43
   is.seekg(2, std::ios_base::cur);                                      // 44
   is.read(reinterpret_cast<char*>(&p->ifdrChasisTemperature_), 2);      // 45
   is.read(reinterpret_cast<char*>(&p->ifdrFpgaTemperature_), 2);        // 46
   is.read(reinterpret_cast<char*>(&p->ntpStatus_), 2);                  // 47
   is.seekg(10, std::ios_base::cur);                                  // 48-52
   is.read(reinterpret_cast<char*>(&p->ipcStatus_), 2);               // 53
   is.read(reinterpret_cast<char*>(&p->commandedChannelControl_), 2); // 54
   is.seekg(6, std::ios_base::cur);                                   // 55-57

   // AME
   is.read(reinterpret_cast<char*>(&p->polarization_), 2);           // 58
   is.read(reinterpret_cast<char*>(&p->ameInternalTemperature_), 4); // 59-60
   is.read(reinterpret_cast<char*>(&p->ameReceiverModuleTemperature_),
           4); // 61-62
   is.read(reinterpret_cast<char*>(&p->ameBiteCalModuleTemperature_),
           4); // 63-64
   is.read(reinterpret_cast<char*>(&p->amePeltierPulseWidthModulation_),
           2);                                                     // 65
   is.read(reinterpret_cast<char*>(&p->amePeltierStatus_), 2);     // 66
   is.read(reinterpret_cast<char*>(&p->ameADConverterStatus_), 2); // 67
   is.read(reinterpret_cast<char*>(&p->ameState_), 2);             // 68
   is.read(reinterpret_cast<char*>(&p->ame3_3VPsVoltage_), 4);     // 69-70
   is.read(reinterpret_cast<char*>(&p->ame5VPsVoltage_), 4);       // 71-72
   is.read(reinterpret_cast<char*>(&p->ame6_5VPsVoltage_), 4);     // 73-74
   is.read(reinterpret_cast<char*>(&p->ame15VPsVoltage_), 4);      // 75-76
   is.read(reinterpret_cast<char*>(&p->ame48VPsVoltage_), 4);      // 77-78
   is.read(reinterpret_cast<char*>(&p->ameStaloPower_), 4);        // 79-80
   is.read(reinterpret_cast<char*>(&p->peltierCurrent_), 4);       // 81-82
   is.read(reinterpret_cast<char*>(&p->adcCalibrationReferenceVoltage_),
           4);                                               // 83-84
   is.read(reinterpret_cast<char*>(&p->ameMode_), 2);        // 85
   is.read(reinterpret_cast<char*>(&p->amePeltierMode_), 2); // 86
   is.read(reinterpret_cast<char*>(&p->amePeltierInsideFanCurrent_),
           4); // 87-88
   is.read(reinterpret_cast<char*>(&p->amePeltierOutsideFanCurrent_),
           4); // 89-90
   is.read(reinterpret_cast<char*>(&p->horizontalTrLimiterVoltage_),
           4);                                                         // 91-92
   is.read(reinterpret_cast<char*>(&p->verticalTrLimiterVoltage_), 4); // 93-94
   is.read(reinterpret_cast<char*>(&p->adcCalibrationOffsetVoltage_),
           4); // 95-96
   is.read(reinterpret_cast<char*>(&p->adcCalibrationGainCorrection_),
           4); // 97-98

   // RCP/SPIP Power Button Status
   is.read(reinterpret_cast<char*>(&p->rcpStatus_), 2);        // 99
   is.read(&p->rcpString_[0], 16);                             // 100-107
   is.read(reinterpret_cast<char*>(&p->spipPowerButtons_), 2); // 108
   is.seekg(4, std::ios_base::cur);                            // 109-110

   // Power
   is.read(reinterpret_cast<char*>(&p->masterPowerAdministratorLoad_),
           4); // 111-112
   is.read(reinterpret_cast<char*>(&p->expansionPowerAdministratorLoad_),
           4);                       // 113-114
   is.seekg(44, std::ios_base::cur); // 115-136

   // Transmitter
   is.read(reinterpret_cast<char*>(&p->_5VdcPs_), 2);                    // 137
   is.read(reinterpret_cast<char*>(&p->_15VdcPs_), 2);                   // 138
   is.read(reinterpret_cast<char*>(&p->_28VdcPs_), 2);                   // 139
   is.read(reinterpret_cast<char*>(&p->neg15VdcPs_), 2);                 // 140
   is.read(reinterpret_cast<char*>(&p->_45VdcPs_), 2);                   // 141
   is.read(reinterpret_cast<char*>(&p->filamentPsVoltage_), 2);          // 142
   is.read(reinterpret_cast<char*>(&p->vacuumPumpPsVoltage_), 2);        // 143
   is.read(reinterpret_cast<char*>(&p->focusCoilPsVoltage_), 2);         // 144
   is.read(reinterpret_cast<char*>(&p->filamentPs_), 2);                 // 145
   is.read(reinterpret_cast<char*>(&p->klystronWarmup_), 2);             // 146
   is.read(reinterpret_cast<char*>(&p->transmitterAvailable_), 2);       // 147
   is.read(reinterpret_cast<char*>(&p->wgSwitchPosition_), 2);           // 148
   is.read(reinterpret_cast<char*>(&p->wgPfnTransferInterlock_), 2);     // 149
   is.read(reinterpret_cast<char*>(&p->maintenanceMode_), 2);            // 150
   is.read(reinterpret_cast<char*>(&p->maintenanceRequired_), 2);        // 151
   is.read(reinterpret_cast<char*>(&p->pfnSwitchPosition_), 2);          // 152
   is.read(reinterpret_cast<char*>(&p->modulatorOverload_), 2);          // 153
   is.read(reinterpret_cast<char*>(&p->modulatorInvCurrent_), 2);        // 154
   is.read(reinterpret_cast<char*>(&p->modulatorSwitchFail_), 2);        // 155
   is.read(reinterpret_cast<char*>(&p->mainPowerVoltage_), 2);           // 156
   is.read(reinterpret_cast<char*>(&p->chargingSystemFail_), 2);         // 157
   is.read(reinterpret_cast<char*>(&p->inverseDiodeCurrent_), 2);        // 158
   is.read(reinterpret_cast<char*>(&p->triggerAmplifier_), 2);           // 159
   is.read(reinterpret_cast<char*>(&p->circulatorTemperature_), 2);      // 160
   is.read(reinterpret_cast<char*>(&p->spectrumFilterPressure_), 2);     // 161
   is.read(reinterpret_cast<char*>(&p->wgArcVswr_), 2);                  // 162
   is.read(reinterpret_cast<char*>(&p->cabinetInterlock_), 2);           // 163
   is.read(reinterpret_cast<char*>(&p->cabinetAirTemperature_), 2);      // 164
   is.read(reinterpret_cast<char*>(&p->cabinetAirflow_), 2);             // 165
   is.read(reinterpret_cast<char*>(&p->klystronCurrent_), 2);            // 166
   is.read(reinterpret_cast<char*>(&p->klystronFilamentCurrent_), 2);    // 167
   is.read(reinterpret_cast<char*>(&p->klystronVacionCurrent_), 2);      // 168
   is.read(reinterpret_cast<char*>(&p->klystronAirTemperature_), 2);     // 169
   is.read(reinterpret_cast<char*>(&p->klystronAirflow_), 2);            // 170
   is.read(reinterpret_cast<char*>(&p->modulatorSwitchMaintenance_), 2); // 171
   is.read(reinterpret_cast<char*>(&p->postChargeRegulatorMaintenance_),
           2);                                                            // 172
   is.read(reinterpret_cast<char*>(&p->wgPressureHumidity_), 2);          // 173
   is.read(reinterpret_cast<char*>(&p->transmitterOvervoltage_), 2);      // 174
   is.read(reinterpret_cast<char*>(&p->transmitterOvercurrent_), 2);      // 175
   is.read(reinterpret_cast<char*>(&p->focusCoilCurrent_), 2);            // 176
   is.read(reinterpret_cast<char*>(&p->focusCoilAirflow_), 2);            // 177
   is.read(reinterpret_cast<char*>(&p->oilTemperature_), 2);              // 178
   is.read(reinterpret_cast<char*>(&p->prfLimit_), 2);                    // 179
   is.read(reinterpret_cast<char*>(&p->transmitterOilLevel_), 2);         // 180
   is.read(reinterpret_cast<char*>(&p->transmitterBatteryCharging_), 2);  // 181
   is.read(reinterpret_cast<char*>(&p->highVoltageStatus_), 2);           // 182
   is.read(reinterpret_cast<char*>(&p->transmitterRecyclingSummary_), 2); // 183
   is.read(reinterpret_cast<char*>(&p->transmitterInoperable_), 2);       // 184
   is.read(reinterpret_cast<char*>(&p->transmitterAirFilter_), 2);        // 185
   is.read(reinterpret_cast<char*>(&p->zeroTestBit_[0]),
           p->zeroTestBit_.size() * 2); // 186-193
   is.read(reinterpret_cast<char*>(&p->oneTestBit_[0]),
           p->oneTestBit_.size() * 2);                          // 194-201
   is.read(reinterpret_cast<char*>(&p->xmtrSpipInterface_), 2); // 202
   is.read(reinterpret_cast<char*>(&p->transmitterSummaryStatus_), 2); // 203
   is.seekg(2, std::ios_base::cur);                                    // 204
   is.read(reinterpret_cast<char*>(&p->transmitterRfPower_), 4);      // 205-206
   is.read(reinterpret_cast<char*>(&p->horizontalXmtrPeakPower_), 4); // 207-208
   is.read(reinterpret_cast<char*>(&p->xmtrPeakPower_), 4);           // 209-210
   is.read(reinterpret_cast<char*>(&p->verticalXmtrPeakPower_), 4);   // 211-212
   is.read(reinterpret_cast<char*>(&p->xmtrRfAvgPower_), 4);          // 213-214
   is.seekg(4, std::ios_base::cur);                                   // 215-216
   is.read(reinterpret_cast<char*>(&p->xmtrRecycleCount_), 4);        // 217-218
   is.read(reinterpret_cast<char*>(&p->receiverBias_), 4);            // 219-220
   is.read(reinterpret_cast<char*>(&p->transmitImbalance_), 4);       // 221-222
   is.read(reinterpret_cast<char*>(&p->xmtrPowerMeterZero_), 4);      // 223-224
   is.seekg(8, std::ios_base::cur);                                   // 225-228

   // Tower/Utilities
   is.read(reinterpret_cast<char*>(&p->acUnit1CompressorShutOff_), 2); // 229
   is.read(reinterpret_cast<char*>(&p->acUnit2CompressorShutOff_), 2); // 230
   is.read(reinterpret_cast<char*>(&p->generatorMaintenanceRequired_),
           2);                                                          // 231
   is.read(reinterpret_cast<char*>(&p->generatorBatteryVoltage_), 2);   // 232
   is.read(reinterpret_cast<char*>(&p->generatorEngine_), 2);           // 233
   is.read(reinterpret_cast<char*>(&p->generatorVoltFrequency_), 2);    // 234
   is.read(reinterpret_cast<char*>(&p->powerSource_), 2);               // 235
   is.read(reinterpret_cast<char*>(&p->transitionalPowerSource_), 2);   // 236
   is.read(reinterpret_cast<char*>(&p->generatorAutoRunOffSwitch_), 2); // 237
   is.read(reinterpret_cast<char*>(&p->aircraftHazardLighting_), 2);    // 238
   is.seekg(22, std::ios_base::cur); // 239-249

   // Equipment Shelter
   is.read(reinterpret_cast<char*>(&p->equipmentShelterFireDetectionSystem_),
           2);                                                           // 250
   is.read(reinterpret_cast<char*>(&p->equipmentShelterFireSmoke_), 2);  // 251
   is.read(reinterpret_cast<char*>(&p->generatorShelterFireSmoke_), 2);  // 252
   is.read(reinterpret_cast<char*>(&p->utilityVoltageFrequency_), 2);    // 253
   is.read(reinterpret_cast<char*>(&p->siteSecurityAlarm_), 2);          // 254
   is.read(reinterpret_cast<char*>(&p->securityEquipment_), 2);          // 255
   is.read(reinterpret_cast<char*>(&p->securitySystem_), 2);             // 256
   is.read(reinterpret_cast<char*>(&p->receiverConnectedToAntenna_), 2); // 257
   is.read(reinterpret_cast<char*>(&p->radomeHatch_), 2);                // 258
   is.read(reinterpret_cast<char*>(&p->acUnit1FilterDirty_), 2);         // 259
   is.read(reinterpret_cast<char*>(&p->acUnit2FilterDirty_), 2);         // 260
   is.read(reinterpret_cast<char*>(&p->equipmentShelterTemperature_),
           4); // 261-262
   is.read(reinterpret_cast<char*>(&p->outsideAmbientTemperature_),
           4); // 263-264
   is.read(reinterpret_cast<char*>(&p->transmitterLeavingAirTemp_),
           4);                                                        // 265-266
   is.read(reinterpret_cast<char*>(&p->acUnit1DischargeAirTemp_), 4); // 267-268
   is.read(reinterpret_cast<char*>(&p->generatorShelterTemperature_),
           4);                                                        // 269-270
   is.read(reinterpret_cast<char*>(&p->radomeAirTemperature_), 4);    // 271-272
   is.read(reinterpret_cast<char*>(&p->acUnit2DischargeAirTemp_), 4); // 273-274
   is.read(reinterpret_cast<char*>(&p->spip15VPs_), 4);               // 275-276
   is.read(reinterpret_cast<char*>(&p->spipNeg15VPs_), 4);            // 277-278
   is.read(reinterpret_cast<char*>(&p->spip28VPsStatus_), 2);         // 279
   is.seekg(2, std::ios_base::cur);                                   // 280
   is.read(reinterpret_cast<char*>(&p->spip5VPs_), 4);                // 281-282
   is.read(reinterpret_cast<char*>(&p->convertedGeneratorFuelLevel_), 2); // 283
   is.seekg(32, std::ios_base::cur); // 284-299

   // Antenna/Pedestal
   is.read(reinterpret_cast<char*>(&p->elevationPosDeadLimit_), 2);    // 300
   is.read(reinterpret_cast<char*>(&p->_150VOvervoltage_), 2);         // 301
   is.read(reinterpret_cast<char*>(&p->_150VUndervoltage_), 2);        // 302
   is.read(reinterpret_cast<char*>(&p->elevationServoAmpInhibit_), 2); // 303
   is.read(reinterpret_cast<char*>(&p->elevationServoAmpShortCircuit_),
           2);                                                            // 304
   is.read(reinterpret_cast<char*>(&p->elevationServoAmpOvertemp_), 2);   // 305
   is.read(reinterpret_cast<char*>(&p->elevationMotorOvertemp_), 2);      // 306
   is.read(reinterpret_cast<char*>(&p->elevationStowPin_), 2);            // 307
   is.read(reinterpret_cast<char*>(&p->elevationHousing5VPs_), 2);        // 308
   is.read(reinterpret_cast<char*>(&p->elevationNegDeadLimit_), 2);       // 309
   is.read(reinterpret_cast<char*>(&p->elevationPosNormalLimit_), 2);     // 310
   is.read(reinterpret_cast<char*>(&p->elevationNegNormalLimit_), 2);     // 311
   is.read(reinterpret_cast<char*>(&p->elevationEncoderLight_), 2);       // 312
   is.read(reinterpret_cast<char*>(&p->elevationGearboxOil_), 2);         // 313
   is.read(reinterpret_cast<char*>(&p->elevationHandwheel_), 2);          // 314
   is.read(reinterpret_cast<char*>(&p->elevationAmpPs_), 2);              // 315
   is.read(reinterpret_cast<char*>(&p->azimuthServoAmpInhibit_), 2);      // 316
   is.read(reinterpret_cast<char*>(&p->azimuthServoAmpShortCircuit_), 2); // 317
   is.read(reinterpret_cast<char*>(&p->azimuthServoAmpOvertemp_), 2);     // 318
   is.read(reinterpret_cast<char*>(&p->azimuthMotorOvertemp_), 2);        // 319
   is.read(reinterpret_cast<char*>(&p->azimuthStowPin_), 2);              // 320
   is.read(reinterpret_cast<char*>(&p->azimuthHousing5VPs_), 2);          // 321
   is.read(reinterpret_cast<char*>(&p->azimuthEncoderLight_), 2);         // 322
   is.read(reinterpret_cast<char*>(&p->azimuthGearboxOil_), 2);           // 323
   is.read(reinterpret_cast<char*>(&p->azimuthBullGearOil_), 2);          // 324
   is.read(reinterpret_cast<char*>(&p->azimuthHandwheel_), 2);            // 325
   is.read(reinterpret_cast<char*>(&p->azimuthServoAmpPs_), 2);           // 326
   is.read(reinterpret_cast<char*>(&p->servo_), 2);                       // 327
   is.read(reinterpret_cast<char*>(&p->pedestalInterlockSwitch_), 2);     // 328
   is.seekg(24, std::ios_base::cur); // 329-340

   // RF Generator/Receiver
   is.read(reinterpret_cast<char*>(&p->cohoClock_), 2); // 341
   is.read(reinterpret_cast<char*>(&p->rfGeneratorFrequencySelectOscillator_),
           2);                                                            // 342
   is.read(reinterpret_cast<char*>(&p->rfGeneratorRfStalo_), 2);          // 343
   is.read(reinterpret_cast<char*>(&p->rfGeneratorPhaseShiftedCoho_), 2); // 344
   is.read(reinterpret_cast<char*>(&p->_9VReceiverPs_), 2);               // 345
   is.read(reinterpret_cast<char*>(&p->_5VReceiverPs_), 2);               // 346
   is.read(reinterpret_cast<char*>(&p->_18VReceiverPs_), 2);              // 347
   is.read(reinterpret_cast<char*>(&p->neg9VReceiverPs_), 2);             // 348
   is.read(reinterpret_cast<char*>(&p->_5VSingleChannelRdaiuPs_), 2);     // 349
   is.seekg(2, std::ios_base::cur);                                       // 350
   is.read(reinterpret_cast<char*>(&p->horizontalShortPulseNoise_),
           4); // 351-352
   is.read(reinterpret_cast<char*>(&p->horizontalLongPulseNoise_),
           4); // 353-354
   is.read(reinterpret_cast<char*>(&p->horizontalNoiseTemperature_),
           4);                                                        // 355-356
   is.read(reinterpret_cast<char*>(&p->verticalShortPulseNoise_), 4); // 357-358
   is.read(reinterpret_cast<char*>(&p->verticalLongPulseNoise_), 4);  // 359-360
   is.read(reinterpret_cast<char*>(&p->verticalNoiseTemperature_),
           4); // 361-362

   // Calibration
   is.read(reinterpret_cast<char*>(&p->horizontalLinearity_), 4);    // 363-364
   is.read(reinterpret_cast<char*>(&p->horizontalDynamicRange_), 4); // 365-366
   is.read(reinterpret_cast<char*>(&p->horizontalDeltaDbz0_), 4);    // 367-368
   is.read(reinterpret_cast<char*>(&p->verticalDeltaDbz0_), 4);      // 369-370
   is.read(reinterpret_cast<char*>(&p->kdPeakMeasured_), 4);         // 371-372
   is.seekg(4, std::ios_base::cur);                                  // 373-374
   is.read(reinterpret_cast<char*>(&p->shortPulseHorizontalDbz0_),
           4);                                                        // 375-376
   is.read(reinterpret_cast<char*>(&p->longPulseHorizontalDbz0_), 4); // 377-378
   is.read(reinterpret_cast<char*>(&p->velocityProcessed_), 2);       // 379
   is.read(reinterpret_cast<char*>(&p->widthProcessed_), 2);          // 380
   is.read(reinterpret_cast<char*>(&p->velocityRfGen_), 2);           // 381
   is.read(reinterpret_cast<char*>(&p->widthRfGen_), 2);              // 382
   is.read(reinterpret_cast<char*>(&p->horizontalI0_), 4);            // 383-384
   is.read(reinterpret_cast<char*>(&p->verticalI0_), 4);              // 385-386
   is.read(reinterpret_cast<char*>(&p->verticalDynamicRange_), 4);    // 387-388
   is.read(reinterpret_cast<char*>(&p->shortPulseVerticalDbz0_), 4);  // 389-390
   is.read(reinterpret_cast<char*>(&p->longPulseVerticalDbz0_), 4);   // 391-392
   is.seekg(8, std::ios_base::cur);                                   // 393-396
   is.read(reinterpret_cast<char*>(&p->horizontalPowerSense_), 4);    // 397-398
   is.read(reinterpret_cast<char*>(&p->verticalPowerSense_), 4);      // 399-400
   is.read(reinterpret_cast<char*>(&p->zdrOffset_), 4);               // 401-402
   is.seekg(12, std::ios_base::cur);                                  // 403-408
   is.read(reinterpret_cast<char*>(&p->clutterSuppressionDelta_), 4); // 409-410
   is.read(reinterpret_cast<char*>(&p->clutterSuppressionUnfilteredPower_),
           4); // 411-412
   is.read(reinterpret_cast<char*>(&p->clutterSuppressionFilteredPower_),
           4);                                                  // 413-414
   is.seekg(20, std::ios_base::cur);                            // 415-424
   is.read(reinterpret_cast<char*>(&p->verticalLinearity_), 4); // 425-426
   is.seekg(8, std::ios_base::cur);                             // 427-430

   // File Status
   is.read(reinterpret_cast<char*>(&p->stateFileReadStatus_), 2);      // 431
   is.read(reinterpret_cast<char*>(&p->stateFileWriteStatus_), 2);     // 432
   is.read(reinterpret_cast<char*>(&p->bypassMapFileReadStatus_), 2);  // 433
   is.read(reinterpret_cast<char*>(&p->bypassMapFileWriteStatus_), 2); // 434
   is.seekg(4, std::ios_base::cur); // 435-436
   is.read(reinterpret_cast<char*>(&p->currentAdaptationFileReadStatus_),
           2); // 437
   is.read(reinterpret_cast<char*>(&p->currentAdaptationFileWriteStatus_),
           2);                                                          // 438
   is.read(reinterpret_cast<char*>(&p->censorZoneFileReadStatus_), 2);  // 439
   is.read(reinterpret_cast<char*>(&p->censorZoneFileWriteStatus_), 2); // 440
   is.read(reinterpret_cast<char*>(&p->remoteVcpFileReadStatus_), 2);   // 441
   is.read(reinterpret_cast<char*>(&p->remoteVcpFileWriteStatus_), 2);  // 442
   is.read(reinterpret_cast<char*>(&p->baselineAdaptationFileReadStatus_),
           2);                                                    // 443
   is.read(reinterpret_cast<char*>(&p->readStatusOfPrfSets_), 2); // 444
   is.read(reinterpret_cast<char*>(&p->clutterFilterMapFileReadStatus_),
           2); // 445
   is.read(reinterpret_cast<char*>(&p->clutterFilterMapFileWriteStatus_),
           2);                                                    // 446
   is.read(reinterpret_cast<char*>(&p->generalDiskIoError_), 2);  // 447
   is.read(reinterpret_cast<char*>(&p->rspStatus_), 1);           // 448
   is.seekg(1, std::ios_base::cur);                               // 448
   is.read(reinterpret_cast<char*>(&p->cpu1Temperature_), 1);     // 449
   is.read(reinterpret_cast<char*>(&p->cpu2Temperature_), 1);     // 449
   is.read(reinterpret_cast<char*>(&p->rspMotherboardPower_), 2); // 450
   is.seekg(20, std::ios_base::cur);                              // 451-460

   // Device Status
   is.read(reinterpret_cast<char*>(&p->spipCommStatus_), 2); // 461
   is.read(reinterpret_cast<char*>(&p->hciCommStatus_), 2);  // 462
   is.seekg(2, std::ios_base::cur);                          // 463
   is.read(reinterpret_cast<char*>(&p->signalProcessorCommandStatus_),
           2);                                                       // 464
   is.read(reinterpret_cast<char*>(&p->ameCommunicationStatus_), 2); // 465
   is.read(reinterpret_cast<char*>(&p->rmsLinkStatus_), 2);          // 466
   is.read(reinterpret_cast<char*>(&p->rpgLinkStatus_), 2);          // 467
   is.read(reinterpret_cast<char*>(&p->interpanelLinkStatus_), 2);   // 468
   is.read(reinterpret_cast<char*>(&p->performanceCheckTime_), 4);   // 469
   is.seekg(18, std::ios_base::cur);                                 // 471-479
   is.read(reinterpret_cast<char*>(&p->version_), 2);                // 480

   bytesRead += 960;

   // Communications
   p->loopBackSetStatus_       = ntohs(p->loopBackSetStatus_);
   p->t1OutputFrames_          = ntohl(p->t1OutputFrames_);
   p->t1InputFrames_           = ntohl(p->t1InputFrames_);
   p->routerMemoryUsed_        = ntohl(p->routerMemoryUsed_);
   p->routerMemoryFree_        = ntohl(p->routerMemoryFree_);
   p->routerMemoryUtilization_ = ntohs(p->routerMemoryUtilization_);
   p->routeToRpg_              = ntohs(p->routeToRpg_);
   p->t1PortStatus_            = ntohs(p->t1PortStatus_);
   p->routerDedicatedEthernetPortStatus_ =
      ntohs(p->routerDedicatedEthernetPortStatus_);
   p->routerCommercialEthernetPortStatus_ =
      ntohs(p->routerCommercialEthernetPortStatus_);
   p->csu24HrErroredSeconds_         = ntohl(p->csu24HrErroredSeconds_);
   p->csu24HrSeverelyErroredSeconds_ = ntohl(p->csu24HrSeverelyErroredSeconds_);
   p->csu24HrSeverelyErroredFramingSeconds_ =
      ntohl(p->csu24HrSeverelyErroredFramingSeconds_);
   p->csu24HrUnavailableSeconds_    = ntohl(p->csu24HrUnavailableSeconds_);
   p->csu24HrControlledSlipSeconds_ = ntohl(p->csu24HrControlledSlipSeconds_);
   p->csu24HrPathCodingViolations_  = ntohl(p->csu24HrPathCodingViolations_);
   p->csu24HrLineErroredSeconds_    = ntohl(p->csu24HrLineErroredSeconds_);
   p->csu24HrBurstyErroredSeconds_  = ntohl(p->csu24HrBurstyErroredSeconds_);
   p->csu24HrDegradedMinutes_       = ntohl(p->csu24HrDegradedMinutes_);
   p->lanSwitchCpuUtilization_      = ntohl(p->lanSwitchCpuUtilization_);
   p->lanSwitchMemoryUtilization_   = ntohs(p->lanSwitchMemoryUtilization_);
   p->ifdrChasisTemperature_        = ntohs(p->ifdrChasisTemperature_);
   p->ifdrFpgaTemperature_          = ntohs(p->ifdrFpgaTemperature_);
   p->ntpStatus_                    = ntohs(p->ntpStatus_);
   p->ipcStatus_                    = ntohs(p->ipcStatus_);
   p->commandedChannelControl_      = ntohs(p->commandedChannelControl_);

   // AME
   p->polarization_           = ntohs(p->polarization_);
   p->ameInternalTemperature_ = SwapFloat(p->ameInternalTemperature_);
   p->ameReceiverModuleTemperature_ =
      SwapFloat(p->ameReceiverModuleTemperature_);
   p->ameBiteCalModuleTemperature_ = SwapFloat(p->ameBiteCalModuleTemperature_);
   p->amePeltierPulseWidthModulation_ =
      ntohs(p->amePeltierPulseWidthModulation_);
   p->amePeltierStatus_     = ntohs(p->amePeltierStatus_);
   p->ameADConverterStatus_ = ntohs(p->ameADConverterStatus_);
   p->ameState_             = ntohs(p->ameState_);
   p->ame3_3VPsVoltage_     = SwapFloat(p->ame3_3VPsVoltage_);
   p->ame5VPsVoltage_       = SwapFloat(p->ame5VPsVoltage_);
   p->ame6_5VPsVoltage_     = SwapFloat(p->ame6_5VPsVoltage_);
   p->ame15VPsVoltage_      = SwapFloat(p->ame15VPsVoltage_);
   p->ame48VPsVoltage_      = SwapFloat(p->ame48VPsVoltage_);
   p->ameStaloPower_        = SwapFloat(p->ameStaloPower_);
   p->peltierCurrent_       = SwapFloat(p->peltierCurrent_);
   p->adcCalibrationReferenceVoltage_ =
      SwapFloat(p->adcCalibrationReferenceVoltage_);
   p->ameMode_                     = ntohs(p->ameMode_);
   p->amePeltierMode_              = ntohs(p->amePeltierMode_);
   p->amePeltierInsideFanCurrent_  = SwapFloat(p->amePeltierInsideFanCurrent_);
   p->amePeltierOutsideFanCurrent_ = SwapFloat(p->amePeltierOutsideFanCurrent_);
   p->horizontalTrLimiterVoltage_  = SwapFloat(p->horizontalTrLimiterVoltage_);
   p->verticalTrLimiterVoltage_    = SwapFloat(p->verticalTrLimiterVoltage_);
   p->adcCalibrationOffsetVoltage_ = SwapFloat(p->adcCalibrationOffsetVoltage_);
   p->adcCalibrationGainCorrection_ =
      SwapFloat(p->adcCalibrationGainCorrection_);

   // RCP/SPIP Power Button Status
   p->rcpStatus_        = ntohs(p->rcpStatus_);
   p->spipPowerButtons_ = ntohs(p->spipPowerButtons_);

   // Power
   p->masterPowerAdministratorLoad_ =
      SwapFloat(p->masterPowerAdministratorLoad_);
   p->expansionPowerAdministratorLoad_ =
      SwapFloat(p->expansionPowerAdministratorLoad_);

   // Transmitter
   p->_5VdcPs_                    = ntohs(p->_5VdcPs_);
   p->_15VdcPs_                   = ntohs(p->_15VdcPs_);
   p->_28VdcPs_                   = ntohs(p->_28VdcPs_);
   p->neg15VdcPs_                 = ntohs(p->neg15VdcPs_);
   p->_45VdcPs_                   = ntohs(p->_45VdcPs_);
   p->filamentPsVoltage_          = ntohs(p->filamentPsVoltage_);
   p->vacuumPumpPsVoltage_        = ntohs(p->vacuumPumpPsVoltage_);
   p->focusCoilPsVoltage_         = ntohs(p->focusCoilPsVoltage_);
   p->filamentPs_                 = ntohs(p->filamentPs_);
   p->klystronWarmup_             = ntohs(p->klystronWarmup_);
   p->transmitterAvailable_       = ntohs(p->transmitterAvailable_);
   p->wgSwitchPosition_           = ntohs(p->wgSwitchPosition_);
   p->wgPfnTransferInterlock_     = ntohs(p->wgPfnTransferInterlock_);
   p->maintenanceMode_            = ntohs(p->maintenanceMode_);
   p->maintenanceRequired_        = ntohs(p->maintenanceRequired_);
   p->pfnSwitchPosition_          = ntohs(p->pfnSwitchPosition_);
   p->modulatorOverload_          = ntohs(p->modulatorOverload_);
   p->modulatorInvCurrent_        = ntohs(p->modulatorInvCurrent_);
   p->modulatorSwitchFail_        = ntohs(p->modulatorSwitchFail_);
   p->mainPowerVoltage_           = ntohs(p->mainPowerVoltage_);
   p->chargingSystemFail_         = ntohs(p->chargingSystemFail_);
   p->inverseDiodeCurrent_        = ntohs(p->inverseDiodeCurrent_);
   p->triggerAmplifier_           = ntohs(p->triggerAmplifier_);
   p->circulatorTemperature_      = ntohs(p->circulatorTemperature_);
   p->spectrumFilterPressure_     = ntohs(p->spectrumFilterPressure_);
   p->wgArcVswr_                  = ntohs(p->wgArcVswr_);
   p->cabinetInterlock_           = ntohs(p->cabinetInterlock_);
   p->cabinetAirTemperature_      = ntohs(p->cabinetAirTemperature_);
   p->cabinetAirflow_             = ntohs(p->cabinetAirflow_);
   p->klystronCurrent_            = ntohs(p->klystronCurrent_);
   p->klystronFilamentCurrent_    = ntohs(p->klystronFilamentCurrent_);
   p->klystronVacionCurrent_      = ntohs(p->klystronVacionCurrent_);
   p->klystronAirTemperature_     = ntohs(p->klystronAirTemperature_);
   p->klystronAirflow_            = ntohs(p->klystronAirflow_);
   p->modulatorSwitchMaintenance_ = ntohs(p->modulatorSwitchMaintenance_);
   p->postChargeRegulatorMaintenance_ =
      ntohs(p->postChargeRegulatorMaintenance_);
   p->wgPressureHumidity_          = ntohs(p->wgPressureHumidity_);
   p->transmitterOvervoltage_      = ntohs(p->transmitterOvervoltage_);
   p->transmitterOvercurrent_      = ntohs(p->transmitterOvercurrent_);
   p->focusCoilCurrent_            = ntohs(p->focusCoilCurrent_);
   p->focusCoilAirflow_            = ntohs(p->focusCoilAirflow_);
   p->oilTemperature_              = ntohs(p->oilTemperature_);
   p->prfLimit_                    = ntohs(p->prfLimit_);
   p->transmitterOilLevel_         = ntohs(p->transmitterOilLevel_);
   p->transmitterBatteryCharging_  = ntohs(p->transmitterBatteryCharging_);
   p->highVoltageStatus_           = ntohs(p->highVoltageStatus_);
   p->transmitterRecyclingSummary_ = ntohs(p->transmitterRecyclingSummary_);
   p->transmitterInoperable_       = ntohs(p->transmitterInoperable_);
   p->transmitterAirFilter_        = ntohs(p->transmitterAirFilter_);
   SwapArray(p->zeroTestBit_);
   SwapArray(p->oneTestBit_);
   p->xmtrSpipInterface_        = ntohs(p->xmtrSpipInterface_);
   p->transmitterSummaryStatus_ = ntohs(p->transmitterSummaryStatus_);
   p->transmitterRfPower_       = SwapFloat(p->transmitterRfPower_);
   p->horizontalXmtrPeakPower_  = SwapFloat(p->horizontalXmtrPeakPower_);
   p->xmtrPeakPower_            = SwapFloat(p->xmtrPeakPower_);
   p->verticalXmtrPeakPower_    = SwapFloat(p->verticalXmtrPeakPower_);
   p->xmtrRfAvgPower_           = SwapFloat(p->xmtrRfAvgPower_);
   p->xmtrRecycleCount_         = ntohl(p->xmtrRecycleCount_);
   p->receiverBias_             = SwapFloat(p->receiverBias_);
   p->transmitImbalance_        = SwapFloat(p->transmitImbalance_);
   p->xmtrPowerMeterZero_       = SwapFloat(p->xmtrPowerMeterZero_);

   // Tower/Utilities
   p->acUnit1CompressorShutOff_     = ntohs(p->acUnit1CompressorShutOff_);
   p->acUnit2CompressorShutOff_     = ntohs(p->acUnit2CompressorShutOff_);
   p->generatorMaintenanceRequired_ = ntohs(p->generatorMaintenanceRequired_);
   p->generatorBatteryVoltage_      = ntohs(p->generatorBatteryVoltage_);
   p->generatorEngine_              = ntohs(p->generatorEngine_);
   p->generatorVoltFrequency_       = ntohs(p->generatorVoltFrequency_);
   p->powerSource_                  = ntohs(p->powerSource_);
   p->transitionalPowerSource_      = ntohs(p->transitionalPowerSource_);
   p->generatorAutoRunOffSwitch_    = ntohs(p->generatorAutoRunOffSwitch_);
   p->aircraftHazardLighting_       = ntohs(p->aircraftHazardLighting_);

   // Equipment Shelter
   p->equipmentShelterFireDetectionSystem_ =
      ntohs(p->equipmentShelterFireDetectionSystem_);
   p->equipmentShelterFireSmoke_   = ntohs(p->equipmentShelterFireSmoke_);
   p->generatorShelterFireSmoke_   = ntohs(p->generatorShelterFireSmoke_);
   p->utilityVoltageFrequency_     = ntohs(p->utilityVoltageFrequency_);
   p->siteSecurityAlarm_           = ntohs(p->siteSecurityAlarm_);
   p->securityEquipment_           = ntohs(p->securityEquipment_);
   p->securitySystem_              = ntohs(p->securitySystem_);
   p->receiverConnectedToAntenna_  = ntohs(p->receiverConnectedToAntenna_);
   p->radomeHatch_                 = ntohs(p->radomeHatch_);
   p->acUnit1FilterDirty_          = ntohs(p->acUnit1FilterDirty_);
   p->acUnit2FilterDirty_          = ntohs(p->acUnit2FilterDirty_);
   p->equipmentShelterTemperature_ = SwapFloat(p->equipmentShelterTemperature_);
   p->outsideAmbientTemperature_   = SwapFloat(p->outsideAmbientTemperature_);
   p->transmitterLeavingAirTemp_   = SwapFloat(p->transmitterLeavingAirTemp_);
   p->acUnit1DischargeAirTemp_     = SwapFloat(p->acUnit1DischargeAirTemp_);
   p->generatorShelterTemperature_ = SwapFloat(p->generatorShelterTemperature_);
   p->radomeAirTemperature_        = SwapFloat(p->radomeAirTemperature_);
   p->acUnit2DischargeAirTemp_     = SwapFloat(p->acUnit2DischargeAirTemp_);
   p->spip15VPs_                   = SwapFloat(p->spip15VPs_);
   p->spipNeg15VPs_                = SwapFloat(p->spipNeg15VPs_);
   p->spip28VPsStatus_             = ntohs(p->spip28VPsStatus_);
   p->spip5VPs_                    = SwapFloat(p->spip5VPs_);
   p->convertedGeneratorFuelLevel_ = ntohs(p->convertedGeneratorFuelLevel_);

   // Antenna/Pedestal
   p->elevationPosDeadLimit_         = ntohs(p->elevationPosDeadLimit_);
   p->_150VOvervoltage_              = ntohs(p->_150VOvervoltage_);
   p->_150VUndervoltage_             = ntohs(p->_150VUndervoltage_);
   p->elevationServoAmpInhibit_      = ntohs(p->elevationServoAmpInhibit_);
   p->elevationServoAmpShortCircuit_ = ntohs(p->elevationServoAmpShortCircuit_);
   p->elevationServoAmpOvertemp_     = ntohs(p->elevationServoAmpOvertemp_);
   p->elevationMotorOvertemp_        = ntohs(p->elevationMotorOvertemp_);
   p->elevationStowPin_              = ntohs(p->elevationStowPin_);
   p->elevationHousing5VPs_          = ntohs(p->elevationHousing5VPs_);
   p->elevationNegDeadLimit_         = ntohs(p->elevationNegDeadLimit_);
   p->elevationPosNormalLimit_       = ntohs(p->elevationPosNormalLimit_);
   p->elevationNegNormalLimit_       = ntohs(p->elevationNegNormalLimit_);
   p->elevationEncoderLight_         = ntohs(p->elevationEncoderLight_);
   p->elevationGearboxOil_           = ntohs(p->elevationGearboxOil_);
   p->elevationHandwheel_            = ntohs(p->elevationHandwheel_);
   p->elevationAmpPs_                = ntohs(p->elevationAmpPs_);
   p->azimuthServoAmpInhibit_        = ntohs(p->azimuthServoAmpInhibit_);
   p->azimuthServoAmpShortCircuit_   = ntohs(p->azimuthServoAmpShortCircuit_);
   p->azimuthServoAmpOvertemp_       = ntohs(p->azimuthServoAmpOvertemp_);
   p->azimuthMotorOvertemp_          = ntohs(p->azimuthMotorOvertemp_);
   p->azimuthStowPin_                = ntohs(p->azimuthStowPin_);
   p->azimuthHousing5VPs_            = ntohs(p->azimuthHousing5VPs_);
   p->azimuthEncoderLight_           = ntohs(p->azimuthEncoderLight_);
   p->azimuthGearboxOil_             = ntohs(p->azimuthGearboxOil_);
   p->azimuthBullGearOil_            = ntohs(p->azimuthBullGearOil_);
   p->azimuthHandwheel_              = ntohs(p->azimuthHandwheel_);
   p->azimuthServoAmpPs_             = ntohs(p->azimuthServoAmpPs_);
   p->servo_                         = ntohs(p->servo_);
   p->pedestalInterlockSwitch_       = ntohs(p->pedestalInterlockSwitch_);

   // RF Generator/Receiver
   p->cohoClock_ = ntohs(p->cohoClock_);
   p->rfGeneratorFrequencySelectOscillator_ =
      ntohs(p->rfGeneratorFrequencySelectOscillator_);
   p->rfGeneratorRfStalo_          = ntohs(p->rfGeneratorRfStalo_);
   p->rfGeneratorPhaseShiftedCoho_ = ntohs(p->rfGeneratorPhaseShiftedCoho_);
   p->_9VReceiverPs_               = ntohs(p->_9VReceiverPs_);
   p->_5VReceiverPs_               = ntohs(p->_5VReceiverPs_);
   p->_18VReceiverPs_              = ntohs(p->_18VReceiverPs_);
   p->neg9VReceiverPs_             = ntohs(p->neg9VReceiverPs_);
   p->_5VSingleChannelRdaiuPs_     = ntohs(p->_5VSingleChannelRdaiuPs_);
   p->horizontalShortPulseNoise_   = SwapFloat(p->horizontalShortPulseNoise_);
   p->horizontalLongPulseNoise_    = SwapFloat(p->horizontalLongPulseNoise_);
   p->horizontalNoiseTemperature_  = SwapFloat(p->horizontalNoiseTemperature_);
   p->verticalShortPulseNoise_     = SwapFloat(p->verticalShortPulseNoise_);
   p->verticalLongPulseNoise_      = SwapFloat(p->verticalLongPulseNoise_);
   p->verticalNoiseTemperature_    = SwapFloat(p->verticalNoiseTemperature_);

   // Calibration
   p->horizontalLinearity_      = SwapFloat(p->horizontalLinearity_);
   p->horizontalDynamicRange_   = SwapFloat(p->horizontalDynamicRange_);
   p->horizontalDeltaDbz0_      = SwapFloat(p->horizontalDeltaDbz0_);
   p->verticalDeltaDbz0_        = SwapFloat(p->verticalDeltaDbz0_);
   p->kdPeakMeasured_           = SwapFloat(p->kdPeakMeasured_);
   p->shortPulseHorizontalDbz0_ = SwapFloat(p->shortPulseHorizontalDbz0_);
   p->longPulseHorizontalDbz0_  = SwapFloat(p->longPulseHorizontalDbz0_);
   p->velocityProcessed_        = ntohs(p->velocityProcessed_);
   p->widthProcessed_           = ntohs(p->widthProcessed_);
   p->velocityRfGen_            = ntohs(p->velocityRfGen_);
   p->widthRfGen_               = ntohs(p->widthRfGen_);
   p->horizontalI0_             = SwapFloat(p->horizontalI0_);
   p->verticalI0_               = SwapFloat(p->verticalI0_);
   p->verticalDynamicRange_     = SwapFloat(p->verticalDynamicRange_);
   p->shortPulseVerticalDbz0_   = SwapFloat(p->shortPulseVerticalDbz0_);
   p->longPulseVerticalDbz0_    = SwapFloat(p->longPulseVerticalDbz0_);
   p->horizontalPowerSense_     = SwapFloat(p->horizontalPowerSense_);
   p->verticalPowerSense_       = SwapFloat(p->verticalPowerSense_);
   p->zdrOffset_                = SwapFloat(p->zdrOffset_);
   p->clutterSuppressionDelta_  = SwapFloat(p->clutterSuppressionDelta_);
   p->clutterSuppressionUnfilteredPower_ =
      SwapFloat(p->clutterSuppressionUnfilteredPower_);
   p->clutterSuppressionFilteredPower_ =
      SwapFloat(p->clutterSuppressionFilteredPower_);
   p->verticalLinearity_ = SwapFloat(p->verticalLinearity_);

   // File Status
   p->stateFileReadStatus_      = ntohs(p->stateFileReadStatus_);
   p->stateFileWriteStatus_     = ntohs(p->stateFileWriteStatus_);
   p->bypassMapFileReadStatus_  = ntohs(p->bypassMapFileReadStatus_);
   p->bypassMapFileWriteStatus_ = ntohs(p->bypassMapFileWriteStatus_);
   p->currentAdaptationFileReadStatus_ =
      ntohs(p->currentAdaptationFileReadStatus_);
   p->currentAdaptationFileWriteStatus_ =
      ntohs(p->currentAdaptationFileWriteStatus_);
   p->censorZoneFileReadStatus_  = ntohs(p->censorZoneFileReadStatus_);
   p->censorZoneFileWriteStatus_ = ntohs(p->censorZoneFileWriteStatus_);
   p->remoteVcpFileReadStatus_   = ntohs(p->remoteVcpFileReadStatus_);
   p->remoteVcpFileWriteStatus_  = ntohs(p->remoteVcpFileWriteStatus_);
   p->baselineAdaptationFileReadStatus_ =
      ntohs(p->baselineAdaptationFileReadStatus_);
   p->readStatusOfPrfSets_ = ntohs(p->readStatusOfPrfSets_);
   p->clutterFilterMapFileReadStatus_ =
      ntohs(p->clutterFilterMapFileReadStatus_);
   p->clutterFilterMapFileWriteStatus_ =
      ntohs(p->clutterFilterMapFileWriteStatus_);
   p->generalDiskIoError_  = ntohs(p->generalDiskIoError_);
   p->rspMotherboardPower_ = ntohs(p->rspMotherboardPower_);

   // Device Status
   p->spipCommStatus_               = ntohs(p->spipCommStatus_);
   p->hciCommStatus_                = ntohs(p->hciCommStatus_);
   p->signalProcessorCommandStatus_ = ntohs(p->signalProcessorCommandStatus_);
   p->ameCommunicationStatus_       = ntohs(p->ameCommunicationStatus_);
   p->rmsLinkStatus_                = ntohs(p->rmsLinkStatus_);
   p->rpgLinkStatus_                = ntohs(p->rpgLinkStatus_);
   p->interpanelLinkStatus_         = ntohs(p->interpanelLinkStatus_);
   p->performanceCheckTime_         = ntohl(p->performanceCheckTime_);
   p->version_                      = ntohs(p->version_);

   if (!ValidateMessage(is, bytesRead))
   {
      messageValid = false;
   }

   return messageValid;
}

std::shared_ptr<PerformanceMaintenanceData>
PerformanceMaintenanceData::Create(Level2MessageHeader&& header,
                                   std::istream&         is)
{
   std::shared_ptr<PerformanceMaintenanceData> message =
      std::make_shared<PerformanceMaintenanceData>();
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
