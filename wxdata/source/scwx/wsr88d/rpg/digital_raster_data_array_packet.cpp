#include <scwx/wsr88d/rpg/digital_raster_data_array_packet.hpp>
#include <scwx/util/logger.hpp>

#include <istream>
#include <string>

namespace scwx::wsr88d::rpg
{

static const std::string logPrefix_ =
   "scwx::wsr88d::rpg::digital_raster_data_array_packet";
static const auto logger_ = util::Logger::Create(logPrefix_);

class DigitalRasterDataArrayPacket::Impl
{
public:
   struct Row
   {
      std::uint16_t             numberOfBytes_ {0};
      std::vector<std::uint8_t> level_ {};
   };

   explicit Impl() = default;
   ~Impl()         = default;

   Impl(const Impl&)             = delete;
   Impl& operator=(const Impl&)  = delete;
   Impl(const Impl&&)            = delete;
   Impl& operator=(const Impl&&) = delete;

   std::uint16_t packetCode_ {0};
   std::uint16_t iCoordinateStart_ {0};
   std::uint16_t jCoordinateStart_ {0};
   std::uint16_t iScaleFactor_ {0};
   std::uint16_t jScaleFactor_ {0};
   std::uint16_t numberOfCells_ {0};
   std::uint16_t numberOfRows_ {0};
   std::uint16_t numberOfBytesInRow_ {0};

   // Repeat for each row
   std::vector<Row> row_ {};

   std::size_t dataSize_ {0};
};

DigitalRasterDataArrayPacket::DigitalRasterDataArrayPacket() :
    p(std::make_unique<Impl>())
{
}
DigitalRasterDataArrayPacket::~DigitalRasterDataArrayPacket() = default;

DigitalRasterDataArrayPacket::DigitalRasterDataArrayPacket(
   DigitalRasterDataArrayPacket&&) noexcept = default;
DigitalRasterDataArrayPacket& DigitalRasterDataArrayPacket::operator=(
   DigitalRasterDataArrayPacket&&) noexcept = default;

std::uint16_t DigitalRasterDataArrayPacket::packet_code() const
{
   return p->packetCode_;
}

std::uint16_t DigitalRasterDataArrayPacket::i_coordinate_start() const
{
   return p->iCoordinateStart_;
}

std::uint16_t DigitalRasterDataArrayPacket::j_coordinate_start() const
{
   return p->jCoordinateStart_;
}

std::uint16_t DigitalRasterDataArrayPacket::i_scale_factor() const
{
   return p->iScaleFactor_;
}

std::uint16_t DigitalRasterDataArrayPacket::j_scale_factor() const
{
   return p->jScaleFactor_;
}

std::uint16_t DigitalRasterDataArrayPacket::number_of_cells() const
{
   return p->numberOfCells_;
}

std::uint16_t DigitalRasterDataArrayPacket::number_of_rows() const
{
   return p->numberOfRows_;
}

std::uint16_t
DigitalRasterDataArrayPacket::number_of_bytes_in_row(std::uint16_t r) const
{
   return p->row_[r].numberOfBytes_;
}

const std::vector<std::uint8_t>&
DigitalRasterDataArrayPacket::level(std::uint16_t r) const
{
   return p->row_[r].level_;
}

size_t DigitalRasterDataArrayPacket::data_size() const
{
   return p->dataSize_;
}

bool DigitalRasterDataArrayPacket::Parse(std::istream& is)
{
   bool        blockValid = true;
   std::size_t bytesRead  = 0;

   // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)

   is.read(reinterpret_cast<char*>(&p->packetCode_), 2);
   is.read(reinterpret_cast<char*>(&p->iCoordinateStart_), 2);
   is.read(reinterpret_cast<char*>(&p->jCoordinateStart_), 2);
   is.read(reinterpret_cast<char*>(&p->iScaleFactor_), 2);
   is.read(reinterpret_cast<char*>(&p->jScaleFactor_), 2);
   is.read(reinterpret_cast<char*>(&p->numberOfCells_), 2);
   is.read(reinterpret_cast<char*>(&p->numberOfRows_), 2);
   bytesRead += 14;

   p->packetCode_       = ntohs(p->packetCode_);
   p->iCoordinateStart_ = ntohs(p->iCoordinateStart_);
   p->jCoordinateStart_ = ntohs(p->jCoordinateStart_);
   p->iScaleFactor_     = ntohs(p->iScaleFactor_);
   p->jScaleFactor_     = ntohs(p->jScaleFactor_);
   p->numberOfCells_    = ntohs(p->numberOfCells_);
   p->numberOfRows_     = ntohs(p->numberOfRows_);

   if (is.eof())
   {
      logger_->debug("Reached end of file");
      blockValid = false;
   }
   else
   {
      if (p->packetCode_ != 33)
      {
         logger_->warn("Invalid packet code: {}", p->packetCode_);
         blockValid = false;
      }
      if (p->numberOfCells_ < 1 || p->numberOfCells_ > 1840)
      {
         logger_->warn("Invalid number of cells: {}", p->numberOfCells_);
         blockValid = false;
      }
      if (p->numberOfRows_ < 1 || p->numberOfRows_ > 464)
      {
         logger_->warn("Invalid number of rows: {}", p->numberOfRows_);
         blockValid = false;
      }
   }

   if (blockValid)
   {
      p->row_.resize(p->numberOfRows_);

      for (std::uint16_t r = 0; r < p->numberOfRows_; r++)
      {
         auto& row = p->row_[r];

         is.read(reinterpret_cast<char*>(&row.numberOfBytes_), 2);
         bytesRead += 2;

         row.numberOfBytes_ = ntohs(row.numberOfBytes_);

         if (row.numberOfBytes_ < 1 || row.numberOfBytes_ > 1840)
         {
            logger_->warn(
               "Invalid number of bytes: {} (Row {})", row.numberOfBytes_, r);
            blockValid = false;
            break;
         }
         else if (row.numberOfBytes_ < p->numberOfCells_)
         {
            logger_->warn("Number of bytes < number of cells: {} < {} (Row {})",
                          row.numberOfBytes_,
                          p->numberOfCells_,
                          r);
            blockValid = false;
            break;
         }

         // Read raster bins
         const std::size_t dataSize = p->numberOfCells_;
         row.level_.resize(dataSize);
         is.read(reinterpret_cast<char*>(row.level_.data()),
                 static_cast<std::streamsize>(dataSize));

         is.seekg(static_cast<std::streamoff>(row.numberOfBytes_ - dataSize),
                  std::ios_base::cur);
         bytesRead += row.numberOfBytes_;
      }
   }

   // NOLINTEND(cppcoreguidelines-avoid-magic-numbers)

   p->dataSize_ = bytesRead;

   if (!ValidateMessage(is, bytesRead))
   {
      blockValid = false;
   }

   return blockValid;
}

std::shared_ptr<DigitalRasterDataArrayPacket>
DigitalRasterDataArrayPacket::Create(std::istream& is)
{
   std::shared_ptr<DigitalRasterDataArrayPacket> packet =
      std::make_shared<DigitalRasterDataArrayPacket>();

   if (!packet->Parse(is))
   {
      packet.reset();
   }

   return packet;
}

} // namespace scwx::wsr88d::rpg
