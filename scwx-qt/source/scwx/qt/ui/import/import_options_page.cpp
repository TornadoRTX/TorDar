#include <scwx/qt/ui/import/import_options_page.hpp>
#include <scwx/qt/types/settings_types.hpp>
#include <scwx/util/logger.hpp>

#include <set>

#include <QGroupBox>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QVBoxLayout>

namespace scwx::qt::ui::import
{

static const std::string logPrefix_ =
   "scwx::qt::ui::import::import_options_page";
static const auto logger_ = scwx::util::Logger::Create(logPrefix_);

class ImportOptionsPage::Impl
{
public:
   explicit Impl(ImportOptionsPage* self) : self_ {self} {};
   ~Impl() = default;

   Impl(const Impl&)             = delete;
   Impl& operator=(const Impl&)  = delete;
   Impl(const Impl&&)            = delete;
   Impl& operator=(const Impl&&) = delete;

   ImportOptionsPage* self_;

   QLayout* layout_ {};

   QLabel*      importingFromLabel_ {};
   QGroupBox*   optionsGroupBox_ {};
   QLayout*     optionsLayout_ {};
   QListWidget* optionsListWidget_ {};

   std::shared_ptr<zip::ZipStreamReader> settingsFile_ {};
   std::set<types::SettingsType>         selectedTypes_ {};
   std::set<types::SettingsType>         settingsTypes_ {};
};

ImportOptionsPage::ImportOptionsPage(QWidget* parent) :
    QWizardPage(parent), p {std::make_shared<Impl>(this)}
{
   // QObjects are managed by the parent
   // NOLINTBEGIN(cppcoreguidelines-owning-memory)

   p->importingFromLabel_ = new QLabel(this);
   p->optionsGroupBox_    = new QGroupBox(this);
   p->optionsListWidget_  = new QListWidget(this);

   p->optionsGroupBox_->setTitle(tr("Options"));

   // Options group box layout
   p->optionsLayout_ = new QVBoxLayout(this);
   p->optionsLayout_->addWidget(p->optionsListWidget_);
   p->optionsGroupBox_->setLayout(p->optionsLayout_);

   // Overall layout
   p->layout_ = new QVBoxLayout(this);
   p->layout_->addWidget(p->importingFromLabel_);
   p->layout_->addWidget(p->optionsGroupBox_);
   setLayout(p->layout_);

   // NOLINTEND(cppcoreguidelines-owning-memory)

   // Connect signals
   connect(p->optionsListWidget_,
           &QListWidget::itemChanged,
           this,
           [this](const QListWidgetItem* item)
           {
              const auto settingsType = static_cast<types::SettingsType>(
                 item->data(Qt::ItemDataRole::UserRole).toInt());

              if (item->checkState() == Qt::CheckState::Checked)
              {
                 p->selectedTypes_.emplace(settingsType);
              }
              else
              {
                 p->selectedTypes_.erase(settingsType);
              }

              logger_->error("Got here");

              // Inform the wizard that completion may have changed
              Q_EMIT completeChanged();
           });
}

ImportOptionsPage::~ImportOptionsPage() = default;

void ImportOptionsPage::set_settings_file(
   const std::string&                           settingsFilename,
   const std::shared_ptr<zip::ZipStreamReader>& settingsFile)
{
   p->settingsFile_ = settingsFile;

   p->importingFromLabel_->setText(
      tr("Importing from: %1").arg(QString::fromStdString(settingsFilename)));

   // Remove all items from the list widget
   p->optionsListWidget_->clear();
   p->settingsTypes_.clear();
   p->selectedTypes_.clear();

   if (settingsFile != nullptr)
   {
      const auto fileList = settingsFile->ListFiles();
      for (const auto& filename : fileList)
      {
         // Search for settings files
         const auto settingsType = types::GetSettingsTypeFromFilename(filename);
         if (settingsType != types::SettingsType::Unknown)
         {
            p->settingsTypes_.emplace(settingsType);
         }
      }
   }

   // Add settings present in the list widget
   for (const auto& settingsType : p->settingsTypes_)
   {
      const QString name =
         QString::fromStdString(types::GetSettingsTypeName(settingsType));

      // NOLINTNEXTLINE(cppcoreguidelines-owning-memory): Owned by parent
      auto item = new QListWidgetItem(name, p->optionsListWidget_);
      item->setFlags(item->flags() | Qt::ItemFlag::ItemIsUserCheckable);
      item->setCheckState(Qt::CheckState::Checked);
      item->setData(Qt::ItemDataRole::UserRole, static_cast<int>(settingsType));

      p->selectedTypes_.emplace(settingsType);
   }
}

bool ImportOptionsPage::isComplete() const
{
   return !p->selectedTypes_.empty();
}

bool ImportOptionsPage::validatePage()
{
   return false;
}

} // namespace scwx::qt::ui::import
