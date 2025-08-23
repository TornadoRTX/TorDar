#include <scwx/qt/manager/task_manager.hpp>
#include <scwx/network/ntp_client.hpp>
#include <scwx/util/logger.hpp>

namespace scwx::qt::manager::TaskManager
{

static const std::string logPrefix_ = "scwx::qt::manager::task_manager";
static const auto        logger_    = scwx::util::Logger::Create(logPrefix_);

static std::shared_ptr<network::NtpClient> ntpClient_ {};

void Initialize()
{
   logger_->debug("Initialize");

   ntpClient_ = network::NtpClient::Instance();

   ntpClient_->Start();
}

void Shutdown()
{
   logger_->debug("Shutdown");

   ntpClient_->Stop();
}

} // namespace scwx::qt::manager::TaskManager
