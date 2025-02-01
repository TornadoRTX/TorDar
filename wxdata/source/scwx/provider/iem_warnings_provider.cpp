#include <scwx/provider/iem_warnings_provider.hpp>
#include <scwx/util/logger.hpp>

namespace scwx::provider
{

static const std::string logPrefix_ = "scwx::provider::iem_warnings_provider";
static const auto        logger_    = util::Logger::Create(logPrefix_);

class IemWarningsProvider::Impl
{
public:
   explicit Impl()               = default;
   ~Impl()                       = default;
   Impl(const Impl&)             = delete;
   Impl& operator=(const Impl&)  = delete;
   Impl(const Impl&&)            = delete;
   Impl& operator=(const Impl&&) = delete;
};

IemWarningsProvider::IemWarningsProvider() : p(std::make_unique<Impl>()) {}
IemWarningsProvider::~IemWarningsProvider() = default;

IemWarningsProvider::IemWarningsProvider(IemWarningsProvider&&) noexcept =
   default;
IemWarningsProvider&
IemWarningsProvider::operator=(IemWarningsProvider&&) noexcept = default;

} // namespace scwx::provider
