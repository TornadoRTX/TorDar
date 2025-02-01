#include <scwx/qt/settings/settings_interface_base.hpp>

#include <string>

namespace scwx::qt::settings
{

static const std::string logPrefix_ =
   "scwx::qt::settings::settings_interface_base";

class SettingsInterfaceBase::Impl
{
public:
   explicit Impl()               = default;
   ~Impl()                       = default;
   Impl(const Impl&)             = delete;
   Impl& operator=(const Impl&)  = delete;
   Impl(const Impl&&)            = delete;
   Impl& operator=(const Impl&&) = delete;
};

SettingsInterfaceBase::SettingsInterfaceBase() : p(std::make_unique<Impl>()) {}

SettingsInterfaceBase::~SettingsInterfaceBase() = default;

SettingsInterfaceBase::SettingsInterfaceBase(SettingsInterfaceBase&&) noexcept =
   default;

SettingsInterfaceBase&
SettingsInterfaceBase::operator=(SettingsInterfaceBase&&) noexcept = default;

} // namespace scwx::qt::settings
