#include <scwx/qt/map/color_table_layer.hpp>
#include <scwx/qt/gl/shader_program.hpp>
#include <scwx/qt/view/radar_product_view.hpp>
#include <scwx/util/logger.hpp>

#if defined(_MSC_VER)
#   pragma warning(push, 0)
#endif

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#if defined(_MSC_VER)
#   pragma warning(pop)
#endif

namespace scwx::qt::map
{

static const std::string logPrefix_ = "scwx::qt::map::color_table_layer";
static const auto        logger_    = scwx::util::Logger::Create(logPrefix_);

class ColorTableLayer::Impl
{
public:
   explicit Impl() = default;
   ~Impl()         = default;

   Impl(const Impl&)             = delete;
   Impl& operator=(const Impl&)  = delete;
   Impl(const Impl&&)            = delete;
   Impl& operator=(const Impl&&) = delete;

   std::shared_ptr<gl::ShaderProgram> shaderProgram_ {nullptr};

   GLint uMVPMatrixLocation_ {static_cast<GLint>(GL_INVALID_INDEX)};
   std::array<GLuint, 2> vbo_ {GL_INVALID_INDEX};
   GLuint                vao_ {GL_INVALID_INDEX};
   GLuint                texture_ {GL_INVALID_INDEX};

   std::vector<boost::gil::rgba8_pixel_t> colorTable_ {};

   bool colorTableNeedsUpdate_ {true};
};

ColorTableLayer::ColorTableLayer(std::shared_ptr<gl::GlContext> glContext) :
    GenericLayer(std::move(glContext)), p(std::make_unique<Impl>())
{
}
ColorTableLayer::~ColorTableLayer() = default;

void ColorTableLayer::Initialize(const std::shared_ptr<MapContext>& mapContext)
{
   logger_->debug("Initialize()");

   auto glContext = gl_context();

   gl::OpenGLFunctions& gl = glContext->gl();

   // Load and configure overlay shader
   p->shaderProgram_ =
      glContext->GetShaderProgram(":/gl/texture1d.vert", ":/gl/texture1d.frag");

   p->uMVPMatrixLocation_ =
      gl.glGetUniformLocation(p->shaderProgram_->id(), "uMVPMatrix");
   if (p->uMVPMatrixLocation_ == -1)
   {
      logger_->warn("Could not find uMVPMatrix");
   }

   gl.glGenTextures(1, &p->texture_);

   p->shaderProgram_->Use();

   // Generate a vertex array object
   gl.glGenVertexArrays(1, &p->vao_);

   // Generate vertex buffer objects
   gl.glGenBuffers(2, p->vbo_.data());

   gl.glBindVertexArray(p->vao_);

   // Bottom panel
   gl.glBindBuffer(GL_ARRAY_BUFFER, p->vbo_[0]);
   gl.glBufferData(
      GL_ARRAY_BUFFER, sizeof(float) * 6 * 2, nullptr, GL_DYNAMIC_DRAW);

   gl.glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, static_cast<void*>(0));
   gl.glEnableVertexAttribArray(0);

   // Color table panel texture coordinates
   const float textureCoords[6][1] = {{0.0f}, // TL
                                      {0.0f}, // BL
                                      {1.0f}, // TR
                                      //
                                      {0.0f},  // BL
                                      {1.0f},  // TR
                                      {1.0f}}; // BR
   gl.glBindBuffer(GL_ARRAY_BUFFER, p->vbo_[1]);
   gl.glBufferData(
      GL_ARRAY_BUFFER, sizeof(textureCoords), textureCoords, GL_STATIC_DRAW);

   gl.glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, static_cast<void*>(0));
   gl.glEnableVertexAttribArray(1);

   connect(mapContext->radar_product_view().get(),
           &view::RadarProductView::ColorTableLutUpdated,
           this,
           [this]() { p->colorTableNeedsUpdate_ = true; });
}

void ColorTableLayer::Render(
   const std::shared_ptr<MapContext>&            mapContext,
   const QMapLibre::CustomLayerRenderParameters& params)
{
   gl::OpenGLFunctions& gl               = gl_context()->gl();
   auto                 radarProductView = mapContext->radar_product_view();

   if (radarProductView == nullptr || !radarProductView->IsInitialized())
   {
      // Defer rendering until view is initialized
      return;
   }

   glm::mat4 projection = glm::ortho(0.0f,
                                     static_cast<float>(params.width),
                                     0.0f,
                                     static_cast<float>(params.height));

   p->shaderProgram_->Use();

   // Set OpenGL blend mode for transparency
   gl.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   gl.glUniformMatrix4fv(
      p->uMVPMatrixLocation_, 1, GL_FALSE, glm::value_ptr(projection));

   if (p->colorTableNeedsUpdate_)
   {
      p->colorTable_ = radarProductView->color_table_lut();

      gl.glActiveTexture(GL_TEXTURE0);
      gl.glBindTexture(GL_TEXTURE_1D, p->texture_);
      gl.glTexImage1D(GL_TEXTURE_1D,
                      0,
                      GL_RGBA,
                      (GLsizei) p->colorTable_.size(),
                      0,
                      GL_RGBA,
                      GL_UNSIGNED_BYTE,
                      p->colorTable_.data());
      gl.glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      gl.glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      gl.glGenerateMipmap(GL_TEXTURE_1D);
   }

   if (p->colorTable_.size() > 0 && radarProductView->sweep_time() !=
                                       std::chrono::system_clock::time_point())
   {
      // Color table panel vertices
      const float vertexLX       = 0.0f;
      const float vertexRX       = static_cast<float>(params.width);
      const float vertexTY       = 10.0f;
      const float vertexBY       = 0.0f;
      const float vertices[6][2] = {{vertexLX, vertexTY}, // TL
                                    {vertexLX, vertexBY}, // BL
                                    {vertexRX, vertexTY}, // TR
                                    //
                                    {vertexLX, vertexBY},  // BL
                                    {vertexRX, vertexTY},  // TR
                                    {vertexRX, vertexBY}}; // BR

      // Draw vertices
      gl.glBindVertexArray(p->vao_);
      gl.glBindBuffer(GL_ARRAY_BUFFER, p->vbo_[0]);
      gl.glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
      gl.glDrawArrays(GL_TRIANGLES, 0, 6);

      mapContext->set_color_table_margins(QMargins {0, 0, 0, 10});
   }
   else
   {
      mapContext->set_color_table_margins(QMargins {});
   }

   SCWX_GL_CHECK_ERROR();
}

void ColorTableLayer::Deinitialize()
{
   logger_->debug("Deinitialize()");

   gl::OpenGLFunctions& gl = gl_context()->gl();

   gl.glDeleteVertexArrays(1, &p->vao_);
   gl.glDeleteBuffers(2, p->vbo_.data());
   gl.glDeleteTextures(1, &p->texture_);

   p->uMVPMatrixLocation_ = GL_INVALID_INDEX;
   p->vao_                = GL_INVALID_INDEX;
   p->vbo_                = {GL_INVALID_INDEX};
   p->texture_            = GL_INVALID_INDEX;
}

} // namespace scwx::qt::map
