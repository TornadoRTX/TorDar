#include <scwx/qt/gl/gl_context.hpp>
#include <scwx/qt/util/texture_atlas.hpp>
#include <scwx/util/logger.hpp>

#include <boost/container_hash/hash.hpp>

namespace scwx
{
namespace qt
{
namespace gl
{

static const std::string logPrefix_ = "scwx::qt::gl::gl_context";
static const auto        logger_    = scwx::util::Logger::Create(logPrefix_);

class GlContext::Impl
{
public:
   explicit Impl() = default;
   ~Impl()         = default;

   Impl(const Impl&)             = delete;
   Impl& operator=(const Impl&)  = delete;
   Impl(const Impl&&)            = delete;
   Impl& operator=(const Impl&&) = delete;

   void InitializeGL();

   static std::size_t
   GetShaderKey(std::initializer_list<std::pair<GLenum, std::string>> shaders);

   bool glInitialized_ {false};

   std::unordered_map<std::size_t, std::shared_ptr<gl::ShaderProgram>>
              shaderProgramMap_ {};
   std::mutex shaderProgramMutex_ {};

   GLuint     textureAtlas_ {GL_INVALID_INDEX};
   std::mutex textureMutex_ {};

   std::uint64_t textureBufferCount_ {};
};

GlContext::GlContext() : p(std::make_unique<Impl>()) {}
GlContext::~GlContext() = default;

GlContext::GlContext(GlContext&&) noexcept            = default;
GlContext& GlContext::operator=(GlContext&&) noexcept = default;

std::uint64_t GlContext::texture_buffer_count() const
{
   return p->textureBufferCount_;
}

void GlContext::Impl::InitializeGL()
{
   if (glInitialized_)
   {
      return;
   }

   const GLenum error = glewInit();
   if (error != GLEW_OK)
   {
      logger_->error("glewInit failed: {}",
                     reinterpret_cast<const char*>(glewGetErrorString(error)));
   }

   logger_->info("OpenGL Version: {}",
                 reinterpret_cast<const char*>(glGetString(GL_VERSION)));
   logger_->info("OpenGL Vendor: {}",
                 reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
   logger_->info("OpenGL Renderer: {}",
                 reinterpret_cast<const char*>(glGetString(GL_RENDERER)));

   glGenTextures(1, &textureAtlas_);

   glInitialized_ = true;
}

std::shared_ptr<gl::ShaderProgram>
GlContext::GetShaderProgram(const std::string& vertexPath,
                            const std::string& fragmentPath)
{
   return GetShaderProgram(
      {{GL_VERTEX_SHADER, vertexPath}, {GL_FRAGMENT_SHADER, fragmentPath}});
}

std::shared_ptr<gl::ShaderProgram> GlContext::GetShaderProgram(
   std::initializer_list<std::pair<GLenum, std::string>> shaders)
{
   const auto                         key = Impl::GetShaderKey(shaders);
   std::shared_ptr<gl::ShaderProgram> shaderProgram;

   std::unique_lock lock(p->shaderProgramMutex_);

   auto it = p->shaderProgramMap_.find(key);

   if (it == p->shaderProgramMap_.end())
   {
      shaderProgram = std::make_shared<gl::ShaderProgram>();
      shaderProgram->Load(shaders);
      p->shaderProgramMap_[key] = shaderProgram;
   }
   else
   {
      shaderProgram = it->second;
   }

   return shaderProgram;
}

GLuint GlContext::GetTextureAtlas()
{
   p->InitializeGL();

   std::unique_lock lock(p->textureMutex_);

   auto& textureAtlas = util::TextureAtlas::Instance();

   if (p->textureBufferCount_ != textureAtlas.BuildCount())
   {
      p->textureBufferCount_ = textureAtlas.BuildCount();
      textureAtlas.BufferAtlas(p->textureAtlas_);
   }

   return p->textureAtlas_;
}

void GlContext::Initialize()
{
   p->InitializeGL();
}

void GlContext::StartFrame()
{
   glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
   glClear(GL_COLOR_BUFFER_BIT);
}

std::size_t GlContext::Impl::GetShaderKey(
   std::initializer_list<std::pair<GLenum, std::string>> shaders)
{
   std::size_t seed = 0;
   for (auto& shader : shaders)
   {
      boost::hash_combine(seed, shader.first);
      boost::hash_combine(seed, shader.second);
   }
   return seed;
}

} // namespace gl
} // namespace qt
} // namespace scwx
