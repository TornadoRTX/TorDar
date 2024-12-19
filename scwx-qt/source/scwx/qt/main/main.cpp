#define _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING

#include <scwx/qt/config/county_database.hpp>
#include <scwx/qt/config/radar_site.hpp>
#include <scwx/qt/main/main_window.hpp>
#include <scwx/qt/main/process_validation.hpp>
#include <scwx/qt/main/versions.hpp>
#include <scwx/qt/manager/log_manager.hpp>
#include <scwx/qt/manager/radar_product_manager.hpp>
#include <scwx/qt/manager/resource_manager.hpp>
#include <scwx/qt/manager/settings_manager.hpp>
#include <scwx/qt/manager/thread_manager.hpp>
#include <scwx/qt/settings/general_settings.hpp>
#include <scwx/qt/types/qt_types.hpp>
#include <scwx/qt/ui/high_privilege_dialog.hpp>
#include <scwx/qt/ui/setup/setup_wizard.hpp>
#include <scwx/qt/util/check_privilege.hpp>
#include <scwx/network/cpr.hpp>
#include <scwx/util/environment.hpp>
#include <scwx/util/logger.hpp>
#include <scwx/util/threads.hpp>

#include <string>
#include <vector>
#include <filesystem>

#include <aws/core/Aws.h>
#include <boost/asio.hpp>
#include <fmt/format.h>
#include <QApplication>
#include <QStandardPaths>
#include <QStyleHints>
#include <QTranslator>
#include <QPalette>
#include <QStyle>

#define QT6CT_LIBRARY
#include <qt6ct-common/qt6ct.h>
#undef QT6CT_LIBRARY

static const std::string logPrefix_ = "scwx::main";
static const auto        logger_    = scwx::util::Logger::Create(logPrefix_);

static void ConfigureTheme(const std::vector<std::string>& args);
static void OverrideDefaultStyle(const std::vector<std::string>& args);

int main(int argc, char* argv[])
{
   bool disableHighPrivilegeWarning = false;
   bool highPrivilegeChecked        = false;

   // Store arguments
   std::vector<std::string> args {};
   for (int i = 0; i < argc; ++i)
   {
      args.push_back(argv[i]);
   }

   // Initialize logger
   auto& logManager = scwx::qt::manager::LogManager::Instance();
   logManager.Initialize();

   logger_->info("Supercell Wx v{} ({})",
                 scwx::qt::main::kVersionString_,
                 scwx::qt::main::kCommitString_);

   QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts, true);

   QApplication a(argc, argv);

   QCoreApplication::setApplicationName("Supercell Wx");
   scwx::network::cpr::SetUserAgent(
      fmt::format("SupercellWx/{}", scwx::qt::main::kVersionString_));

   // Enable internationalization support
   QTranslator translator;
   if (translator.load(QLocale(), "scwx", "_", ":/i18n"))
   {
      QCoreApplication::installTranslator(&translator);
   }

   if (!scwx::util::GetEnvironment("SCWX_TEST").empty())
   {
      QStandardPaths::setTestModeEnabled(true);
   }

   // Test to see if scwx was run with high privilege
   const std::string appDataPath {
      QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
         .toStdString()};

   // Check if high privilege before writing settings, assuming no settings
   // have been written
   if (!std::filesystem::exists(appDataPath) &&
       scwx::qt::util::is_high_privilege())
   {
      auto dialog =
         scwx::qt::ui::HighPrivilegeDialog(); // TODO does this need cleaned up?
      const int result = dialog.exec();

      disableHighPrivilegeWarning = dialog.disable_high_privilege_message();
      highPrivilegeChecked        = true;

      if (result == QDialog::Rejected)
      {
         // TODO any other cleanup needed here?
         return 0;
      }
   }

   // Start the io_context main loop
   boost::asio::io_context& ioContext = scwx::util::io_context();
   auto                     work      = boost::asio::make_work_guard(ioContext);
   boost::asio::thread_pool threadPool {4};
   boost::asio::post(threadPool,
                     [&]()
                     {
                        while (true)
                        {
                           try
                           {
                              ioContext.run();
                              break; // run() exited normally
                           }
                           catch (std::exception& ex)
                           {
                              // Log exception and continue
                              logger_->error(ex.what());
                           }
                        }
                     });

   // Initialize AWS SDK
   Aws::SDKOptions awsSdkOptions;
   Aws::InitAPI(awsSdkOptions);

   // Initialize application
   logManager.InitializeLogFile();
   scwx::qt::config::RadarSite::Initialize();
   scwx::qt::config::CountyDatabase::Initialize();
   scwx::qt::manager::SettingsManager::Instance().Initialize();
   scwx::qt::manager::ResourceManager::Initialize();

   // Theme
   ConfigureTheme(args);

   // Check process modules for compatibility
   scwx::qt::main::CheckProcessModules();
   auto& generalSettings = scwx::qt::settings::GeneralSettings::Instance();

   if (!highPrivilegeChecked &&
       generalSettings.high_privilege_warning_enabled().GetValue() &&
       scwx::qt::util::is_high_privilege())
   {
      auto dialog =
         scwx::qt::ui::HighPrivilegeDialog(); // TODO does this need cleaned up?
      const int result = dialog.exec();

      disableHighPrivilegeWarning = dialog.disable_high_privilege_message();

      if (result == QDialog::Rejected)
      {
         // Deinitialize application
         scwx::qt::manager::RadarProductManager::Cleanup();

         // Stop Qt Threads
         scwx::qt::manager::ThreadManager::Instance().StopThreads();

         // Gracefully stop the io_context main loop
         work.reset();
         threadPool.join();

         // Shutdown application
         scwx::qt::manager::ResourceManager::Shutdown();
         scwx::qt::manager::SettingsManager::Instance().Shutdown();

         // Shutdown AWS SDK
         Aws::ShutdownAPI(awsSdkOptions);
         return 0;
      }
   }

   // Save high privilege settings
   if (disableHighPrivilegeWarning)
   {
      generalSettings.high_privilege_warning_enabled().SetValue(false);
      scwx::qt::manager::SettingsManager::Instance().SaveSettings();
   }

   // Run initial setup if required
   if (scwx::qt::ui::setup::SetupWizard::IsSetupRequired())
   {
      scwx::qt::ui::setup::SetupWizard w;
      w.show();
      a.exec();
   }

   // Run Qt main loop
   int result;
   {
      scwx::qt::main::MainWindow w;
      w.show();
      result = a.exec();
   }

   // Deinitialize application
   scwx::qt::manager::RadarProductManager::Cleanup();

   // Stop Qt Threads
   scwx::qt::manager::ThreadManager::Instance().StopThreads();

   // Gracefully stop the io_context main loop
   work.reset();
   threadPool.join();

   // Shutdown application
   scwx::qt::manager::ResourceManager::Shutdown();
   scwx::qt::manager::SettingsManager::Instance().Shutdown();

   // Shutdown AWS SDK
   Aws::ShutdownAPI(awsSdkOptions);

   return result;
}

static void ConfigureTheme(const std::vector<std::string>& args)
{
   auto& generalSettings = scwx::qt::settings::GeneralSettings::Instance();

   auto uiStyle =
      scwx::qt::types::GetUiStyle(generalSettings.theme().GetValue());
   auto qtColorScheme = scwx::qt::types::GetQtColorScheme(uiStyle);

   if (uiStyle == scwx::qt::types::UiStyle::Default)
   {
      OverrideDefaultStyle(args);
   }
   else
   {
      QApplication::setStyle(
         QString::fromStdString(scwx::qt::types::GetQtStyleName(uiStyle)));
   }

   QGuiApplication::styleHints()->setColorScheme(qtColorScheme);

   std::optional<std::string> paletteFile;
   if (uiStyle == scwx::qt::types::UiStyle::FusionCustom)
   {
      paletteFile = generalSettings.theme_file().GetValue();
   }
   else
   {
      paletteFile = scwx::qt::types::GetQtPaletteFile(uiStyle);
   }

   if (paletteFile)
   {
      QPalette defaultPalette = QApplication::style()->standardPalette();
      QPalette palette        = Qt6CT::loadColorScheme(
         QString::fromStdString(*paletteFile), defaultPalette);

      if (defaultPalette == palette)
      {
         logger_->warn("Failed to load palette file '{}'", *paletteFile);
      }
      else
      {
         logger_->info("Loaded palette file '{}'", *paletteFile);
      }

      QApplication::setPalette(palette);
   }
}

static void
OverrideDefaultStyle([[maybe_unused]] const std::vector<std::string>& args)
{
#if defined(_WIN32)
   bool hasStyleArgument = false;

   for (int i = 1; i < args.size(); ++i)
   {
      if (args.at(i) == "-style")
      {
         hasStyleArgument = true;
         break;
      }
   }

   // Override the default Windows 11 style unless the user supplies a style
   // argument
   if (!hasStyleArgument)
   {
      QApplication::setStyle("windowsvista");
   }
#endif
}
