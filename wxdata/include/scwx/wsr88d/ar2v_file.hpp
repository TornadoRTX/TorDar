#pragma once

#include <scwx/wsr88d/nexrad_file.hpp>
#include <scwx/wsr88d/rda/generic_radar_data.hpp>
#include <scwx/wsr88d/rda/volume_coverage_pattern_data.hpp>

#include <chrono>
#include <memory>
#include <string>

namespace scwx
{
namespace wsr88d
{

class Ar2vFileImpl;

/**
 * @brief The Archive II file is specified in the Interface Control Document for
 * the Archive II/User, Document Number 2620010H, published by the WSR-88D Radar
 * Operations Center.
 */
class Ar2vFile : public NexradFile
{
public:
   explicit Ar2vFile();
   ~Ar2vFile();

   Ar2vFile(const Ar2vFile&)            = delete;
   Ar2vFile& operator=(const Ar2vFile&) = delete;

   Ar2vFile(Ar2vFile&&) noexcept;
   Ar2vFile& operator=(Ar2vFile&&) noexcept;

   std::uint32_t julian_date() const;
   std::uint32_t milliseconds() const;
   std::string   icao() const;

   std::size_t message_count() const;

   std::chrono::system_clock::time_point start_time() const;
   std::chrono::system_clock::time_point end_time() const;

   std::map<std::uint16_t, std::shared_ptr<rda::ElevationScan>>
                                                         radar_data() const;
   std::shared_ptr<const rda::VolumeCoveragePatternData> vcp_data() const;

   std::tuple<std::shared_ptr<rda::ElevationScan>, float, std::vector<float>>
   GetElevationScan(rda::DataBlockType                    dataBlockType,
                    float                                 elevation,
                    std::chrono::system_clock::time_point time) const;

   bool LoadFile(const std::string& filename);
   bool LoadData(std::istream& is);

   bool LoadLDMRecords(std::istream& is);
   bool IndexFile();

private:
   std::unique_ptr<Ar2vFileImpl> p;
};

} // namespace wsr88d
} // namespace scwx
