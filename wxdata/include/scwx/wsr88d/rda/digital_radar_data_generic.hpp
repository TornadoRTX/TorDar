#pragma once

#include <scwx/wsr88d/rda/generic_radar_data.hpp>

namespace scwx::wsr88d::rda
{

class DigitalRadarDataGeneric : public GenericRadarData
{
public:
   class DataBlock;
   class ElevationDataBlock;
   class MomentDataBlock;
   class RadialDataBlock;
   class VolumeDataBlock;

   explicit DigitalRadarDataGeneric();
   ~DigitalRadarDataGeneric();

   DigitalRadarDataGeneric(const DigitalRadarDataGeneric&)            = delete;
   DigitalRadarDataGeneric& operator=(const DigitalRadarDataGeneric&) = delete;

   DigitalRadarDataGeneric(DigitalRadarDataGeneric&&) noexcept;
   DigitalRadarDataGeneric& operator=(DigitalRadarDataGeneric&&) noexcept;

   [[nodiscard]] std::string           radar_identifier() const;
   [[nodiscard]] std::uint32_t         collection_time() const override;
   [[nodiscard]] std::uint16_t         modified_julian_date() const override;
   [[nodiscard]] std::uint16_t         azimuth_number() const override;
   [[nodiscard]] units::degrees<float> azimuth_angle() const override;
   [[nodiscard]] std::uint8_t          compression_indicator() const;
   [[nodiscard]] std::uint16_t         radial_length() const;
   [[nodiscard]] std::uint8_t          azimuth_resolution_spacing() const;
   [[nodiscard]] std::uint8_t          radial_status() const;
   [[nodiscard]] std::uint16_t         elevation_number() const override;
   [[nodiscard]] std::uint8_t          cut_sector_number() const;
   [[nodiscard]] units::degrees<float> elevation_angle() const;
   [[nodiscard]] std::uint8_t          radial_spot_blanking_status() const;
   [[nodiscard]] std::uint8_t          azimuth_indexing_mode() const;
   [[nodiscard]] std::uint16_t         data_block_count() const;
   [[nodiscard]] std::uint16_t volume_coverage_pattern_number() const override;

   [[nodiscard]] std::shared_ptr<ElevationDataBlock>
                                                  elevation_data_block() const;
   [[nodiscard]] std::shared_ptr<RadialDataBlock> radial_data_block() const;
   [[nodiscard]] std::shared_ptr<VolumeDataBlock> volume_data_block() const;
   [[nodiscard]] std::shared_ptr<GenericRadarData::MomentDataBlock>
   moment_data_block(DataBlockType type) const override;

   bool Parse(std::istream& is) override;

   static std::shared_ptr<DigitalRadarDataGeneric>
   Create(Level2MessageHeader&& header, std::istream& is);

private:
   class Impl;
   std::unique_ptr<Impl> p;
};

class DigitalRadarDataGeneric::DataBlock
{
protected:
   explicit DataBlock(const std::string& dataBlockType,
                      const std::string& dataName);

public:
   virtual ~DataBlock();

   DataBlock(const DataBlock&)            = delete;
   DataBlock& operator=(const DataBlock&) = delete;

protected:
   DataBlock(DataBlock&&) noexcept;
   DataBlock& operator=(DataBlock&&) noexcept;

private:
   class Impl;
   std::unique_ptr<Impl> p;
};

class DigitalRadarDataGeneric::ElevationDataBlock : public DataBlock
{
public:
   explicit ElevationDataBlock(const std::string& dataBlockType,
                               const std::string& dataName);
   ~ElevationDataBlock();

   ElevationDataBlock(const ElevationDataBlock&)            = delete;
   ElevationDataBlock& operator=(const ElevationDataBlock&) = delete;

   ElevationDataBlock(ElevationDataBlock&&) noexcept;
   ElevationDataBlock& operator=(ElevationDataBlock&&) noexcept;

   static std::shared_ptr<ElevationDataBlock>
   Create(const std::string& dataBlockType,
          const std::string& dataName,
          std::istream&      is);

private:
   class Impl;
   std::unique_ptr<Impl> p;

   bool Parse(std::istream& is);
};

class DigitalRadarDataGeneric::MomentDataBlock :
    public DataBlock,
    public GenericRadarData::MomentDataBlock
{
public:
   explicit MomentDataBlock(const std::string& dataBlockType,
                            const std::string& dataName);
   ~MomentDataBlock();

   MomentDataBlock(const MomentDataBlock&)            = delete;
   MomentDataBlock& operator=(const MomentDataBlock&) = delete;

   MomentDataBlock(MomentDataBlock&&) noexcept;
   MomentDataBlock& operator=(MomentDataBlock&&) noexcept;

   [[nodiscard]] std::uint16_t            number_of_data_moment_gates() const override;
   [[nodiscard]] units::kilometers<float> data_moment_range() const override;
   [[nodiscard]] std::int16_t             data_moment_range_raw() const override;
   [[nodiscard]] units::kilometers<float> data_moment_range_sample_interval() const override;
   [[nodiscard]] std::uint16_t            data_moment_range_sample_interval_raw() const override;
   [[nodiscard]] float                    snr_threshold() const;
   [[nodiscard]] std::int16_t             snr_threshold_raw() const override;
   [[nodiscard]] std::uint8_t             data_word_size() const override;
   [[nodiscard]] float                    scale() const override;
   [[nodiscard]] float                    offset() const override;
   [[nodiscard]] const void*              data_moments() const override;

   static std::shared_ptr<MomentDataBlock>
   Create(const std::string& dataBlockType,
          const std::string& dataName,
          std::istream&      is);

private:
   class Impl;
   std::unique_ptr<Impl> p;

   bool Parse(std::istream& is);
};

class DigitalRadarDataGeneric::RadialDataBlock : public DataBlock
{
public:
   explicit RadialDataBlock(const std::string& dataBlockType,
                            const std::string& dataName);
   ~RadialDataBlock();

   RadialDataBlock(const RadialDataBlock&)            = delete;
   RadialDataBlock& operator=(const RadialDataBlock&) = delete;

   RadialDataBlock(RadialDataBlock&&) noexcept;
   RadialDataBlock& operator=(RadialDataBlock&&) noexcept;

   [[nodiscard]] float unambiguous_range() const;

   static std::shared_ptr<RadialDataBlock>
   Create(const std::string& dataBlockType,
          const std::string& dataName,
          std::istream&      is);

private:
   class Impl;
   std::unique_ptr<Impl> p;

   bool Parse(std::istream& is);
};

class DigitalRadarDataGeneric::VolumeDataBlock : public DataBlock
{
public:
   explicit VolumeDataBlock(const std::string& dataBlockType,
                            const std::string& dataName);
   ~VolumeDataBlock();

   VolumeDataBlock(const VolumeDataBlock&)            = delete;
   VolumeDataBlock& operator=(const VolumeDataBlock&) = delete;

   VolumeDataBlock(VolumeDataBlock&&) noexcept;
   VolumeDataBlock& operator=(VolumeDataBlock&&) noexcept;

   [[nodiscard]] float         latitude() const;
   [[nodiscard]] float         longitude() const;
   [[nodiscard]] std::uint16_t volume_coverage_pattern_number() const;

   static std::shared_ptr<VolumeDataBlock>
   Create(const std::string& dataBlockType,
          const std::string& dataName,
          std::istream&      is);

private:
   class Impl;
   std::unique_ptr<Impl> p;

   bool Parse(std::istream& is);
};

} // namespace scwx::wsr88d::rda
