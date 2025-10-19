#include <scwx/zip/zip_stream_writer.hpp>
#include <scwx/util/logger.hpp>

#include <fstream>

#include <zip.h>

namespace scwx::zip
{

static const std::string logPrefix_ = "scwx::zip::zip_stream_writer";
static const auto        logger_    = scwx::util::Logger::Create(logPrefix_);

// Stream state for writing to an ostream
struct WriteStreamState
{
   std::ostream* stream;
   zip_int64_t   offset {0};
   bool          error {false};

   WriteStreamState(std::ostream* s) : stream {s} {}
};

class ZipStreamWriter::Impl
{
public:
   explicit Impl();
   ~Impl();
   Impl(const Impl&)            = delete;
   Impl& operator=(const Impl&) = delete;
   Impl(Impl&&)                 = delete;
   Impl& operator=(Impl&&)      = delete;

   void Initialize(std::ostream& stream);

   template<typename T>
   bool   AddFile(const std::string& filename,
                  const T&           content,
                  zip_flags_t        flags = ZIP_FL_OVERWRITE);
   zip_t* OpenFromOstream(std::ostream& stream, int flags, zip_error_t* error);
   zip_int64_t Write(void* data, zip_uint64_t len, zip_source_cmd_t cmd);

   static zip_int64_t WriteCallback(void*            userdata,
                                    void*            data,
                                    zip_uint64_t     len,
                                    zip_source_cmd_t cmd);

   zip_t*                            archive_ {nullptr};
   std::unique_ptr<WriteStreamState> state_ {nullptr};

   std::ofstream fstream_;
};

ZipStreamWriter::ZipStreamWriter(const std::string& filename) :
    p(std::make_unique<Impl>())
{
   p->fstream_ = std::ofstream {filename, std::ios_base::binary};

   if (p->fstream_.is_open())
   {
      p->Initialize(p->fstream_);
   }
}

ZipStreamWriter::ZipStreamWriter(std::ostream& stream) :
    p(std::make_unique<Impl>())
{
   p->Initialize(stream);
}

ZipStreamWriter::~ZipStreamWriter()
{
   if (p->archive_)
   {
      zip_close(p->archive_);
   }
};

ZipStreamWriter::ZipStreamWriter(ZipStreamWriter&&) noexcept = default;
ZipStreamWriter&
ZipStreamWriter::operator=(ZipStreamWriter&&) noexcept = default;

void ZipStreamWriter::Impl::Initialize(std::ostream& stream)
{
   zip_error_t error;
   archive_ = OpenFromOstream(stream, ZIP_CREATE | ZIP_TRUNCATE, &error);

   if (!archive_)
   {
      logger_->error("Failed to create archive");
   }
}

bool ZipStreamWriter::IsOpen() const
{
   return p->archive_ != nullptr;
}

bool ZipStreamWriter::AddFile(const std::string& filename,
                              const std::string& content)
{
   return p->archive_ ? p->AddFile(filename, content) : false;
}

bool ZipStreamWriter::AddFile(const std::string&               filename,
                              const std::vector<std::uint8_t>& content)
{
   return p->archive_ ? p->AddFile(filename, content) : false;
}

// Callback function for writing to an ostream
zip_int64_t
ZipStreamWriter::Impl::Write(void* data, zip_uint64_t len, zip_source_cmd_t cmd)
{
   auto& ws = state_;

   switch (cmd)
   {
   case ZIP_SOURCE_BEGIN_WRITE:
      ws->stream->clear();
      ws->offset = 0;
      return 0;

   case ZIP_SOURCE_COMMIT_WRITE:
      ws->stream->flush();
      return ws->stream->good() ? 0 : -1;

   case ZIP_SOURCE_ROLLBACK_WRITE:
      return 0;

   case ZIP_SOURCE_WRITE:
   {
      if (ws->error)
      {
         return -1;
      }

      ws->stream->write(static_cast<const char*>(data),
                        static_cast<std::streamsize>(len));

      if (!ws->stream->good())
      {
         ws->error = true;
         return -1;
      }

      ws->offset += static_cast<zip_int64_t>(len);
      return static_cast<zip_int64_t>(len);
   }

   case ZIP_SOURCE_SEEK_WRITE:
   {
      auto*       args       = static_cast<zip_source_args_seek_t*>(data);
      zip_int64_t new_offset = 0;

      switch (args->whence)
      {
      case SEEK_SET:
         new_offset = args->offset;
         break;
      case SEEK_CUR:
         new_offset = ws->offset + args->offset;
         break;
      case SEEK_END:
         ws->stream->seekp(0, std::ios::end);
         new_offset = ws->stream->tellp() + args->offset;
         break;
      default:
         return -1;
      }

      if (new_offset < 0)
      {
         return -1;
      }

      ws->stream->seekp(new_offset, std::ios::beg);
      ws->offset = new_offset;
      return 0;
   }

   case ZIP_SOURCE_TELL_WRITE:
      return ws->offset;

   case ZIP_SOURCE_REMOVE:
      return 0;

   case ZIP_SOURCE_FREE:
      ws.reset();
      return 0;

   case ZIP_SOURCE_ERROR:
   {
      auto* err = static_cast<zip_error_t*>(data);
      zip_error_init(err);
      if (ws->error)
      {
         zip_error_set(err, ZIP_ER_WRITE, 0);
      }
      return sizeof(zip_error_t);
   }

   case ZIP_SOURCE_SUPPORTS:
      return zip_source_make_command_bitmap(ZIP_SOURCE_BEGIN_WRITE,
                                            ZIP_SOURCE_COMMIT_WRITE,
                                            ZIP_SOURCE_ROLLBACK_WRITE,
                                            ZIP_SOURCE_WRITE,
                                            ZIP_SOURCE_SEEK_WRITE,
                                            ZIP_SOURCE_TELL_WRITE,
                                            ZIP_SOURCE_REMOVE,
                                            ZIP_SOURCE_FREE,
                                            ZIP_SOURCE_ERROR,
                                            -1);

   default:
      return -1;
   }
}

// Create a zip archive from an output stream (for writing)
zip_t* ZipStreamWriter::Impl::OpenFromOstream(std::ostream& stream,
                                              int           flags,
                                              zip_error_t*  error)
{
   if (!stream)
   {
      return nullptr;
   }

   state_ = std::make_unique<WriteStreamState>(&stream);
   zip_source_t* src =
      zip_source_function_create(Impl::WriteCallback, this, nullptr);

   if (!src)
   {
      state_.reset();
      return nullptr;
   }

   zip_t* archive = zip_open_from_source(src, flags, error);
   if (!archive)
   {
      zip_source_free(src);
      return nullptr;
   }

   return archive;
}

// Add a file to zip archive
template<typename T>
bool ZipStreamWriter::Impl::AddFile(const std::string& filename,
                                    const T&           content,
                                    zip_flags_t        flags)
{
   zip_source_t* src =
      zip_source_buffer(archive_, content.data(), content.size(), 0);
   if (!src)
   {
      return false;
   }

   zip_int64_t idx = zip_file_add(archive_, filename.c_str(), src, flags);
   if (idx < 0)
   {
      zip_source_free(src);
      return false;
   }

   return true;
}

zip_int64_t ZipStreamWriter::Impl::WriteCallback(void*            userdata,
                                                 void*            data,
                                                 zip_uint64_t     len,
                                                 zip_source_cmd_t cmd)
{

   auto* obj = static_cast<Impl*>(userdata);
   return obj->Write(data, len, cmd);
}

} // namespace scwx::zip
