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

   gl::OpenGLFunctions*  gl_ {nullptr};
   QOpenGLFunctions_3_0* gl30_ {nullptr};

   bool glInitialized_ {false};

   std::unordered_map<std::size_t, std::shared_ptr<gl::ShaderProgram>>
              shaderProgramMap_ {};
   std::mutex shaderProgramMutex_ {};

   GLuint     textureAtlas_ {GL_INVALID_INDEX};
   std::mutex textureMutex_ {};

   std::uint64_t textureBufferCount_ {};
};

GlContext::GlContext() : p(std::make_unique<Impl>()) {}
GlContext::~GlContext() {};

GlContext::GlContext(GlContext&&) noexcept            = default;
GlContext& GlContext::operator=(GlContext&&) noexcept = default;

gl::OpenGLFunctions& GlContext::gl()
{
   return *p->gl_;
}

QOpenGLFunctions_3_0& GlContext::gl30()
{
   return *p->gl30_;
}

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

   // QOpenGLFunctions objects will not be freed. Since "destruction" takes
   // place at the end of program execution, it is OK to intentionally leak
   // these.

   // NOLINTBEGIN(cppcoreguidelines-owning-memory)
   gl_   = new gl::OpenGLFunctions();
   gl30_ = new QOpenGLFunctions_3_0();
   // NOLINTEND(cppcoreguidelines-owning-memory)

   gl_->initializeOpenGLFunctions();
   gl30_->initializeOpenGLFunctions();

   gl_->glGenTextures(1, &textureAtlas_);

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
      shaderProgram = std::make_shared<gl::ShaderProgram>(*p->gl_);
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
      textureAtlas.BufferAtlas(*p->gl_, p->textureAtlas_);
   }

   return p->textureAtlas_;
}

void GlContext::Initialize()
{
   p->InitializeGL();
}

void GlContext::StartFrame()
{
   auto& gl = p->gl_;

   gl->glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
   gl->glClear(GL_COLOR_BUFFER_BIT);
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
