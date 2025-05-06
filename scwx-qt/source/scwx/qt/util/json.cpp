#include <scwx/qt/util/json.hpp>
#include <scwx/util/json.hpp>
#include <scwx/util/logger.hpp>

#include <QFile>
#include <QTextStream>

namespace scwx::qt::util::json
{

static const std::string logPrefix_ = "scwx::qt::util::json";
static const auto        logger_    = scwx::util::Logger::Create(logPrefix_);

static boost::json::value ReadJsonFile(QFile& file);

boost::json::value ReadJsonQFile(const std::string& path)
{
   boost::json::value json;

   if (path.starts_with(":"))
   {
      QFile file(path.c_str());
      json = ReadJsonFile(file);
   }
   else
   {
      json = ::scwx::util::json::ReadJsonFile(path);
   }

   return json;
}

static boost::json::value ReadJsonFile(QFile& file)
{
   boost::json::value json;

   if (file.open(QIODevice::ReadOnly))
   {
      QTextStream jsonStream(&file);
      jsonStream.setEncoding(QStringConverter::Utf8);

      std::string        jsonSource = jsonStream.readAll().toStdString();
      std::istringstream is {jsonSource};

      json = ::scwx::util::json::ReadJsonStream(is);

      file.close();
   }
   else
   {
      logger_->warn("Could not open file for reading: \"{}\"",
                    file.fileName().toStdString());
   }

   return json;
}

} // namespace scwx::qt::util::json
