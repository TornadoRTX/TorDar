#include <scwx/qt/manager/resource_manager.hpp>
#include <scwx/qt/manager/font_manager.hpp>
#include <scwx/qt/model/imgui_context_model.hpp>
#include <scwx/qt/types/texture_types.hpp>
#include <scwx/qt/util/texture_atlas.hpp>
#include <scwx/util/logger.hpp>

#include <execution>
#include <mutex>

#include <imgui.h>

namespace scwx
{
namespace qt
{
namespace manager
{
namespace ResourceManager
{

static const std::string logPrefix_ = "scwx::qt::manager::resource_manager";
static const auto        logger_    = scwx::util::Logger::Create(logPrefix_);

static const size_t atlasWidth  = 2048;
static const size_t atlasHeight = 2048;

static void LoadFonts();
static void LoadTextures();

static const std::vector<std::pair<types::Font, std::string>> fontNames_ {
   {types::Font::din1451alt, ":/res/fonts/din1451alt.ttf"},
   {types::Font::din1451alt_g, ":/res/fonts/din1451alt_g.ttf"},
   {types::Font::Inconsolata_Regular, ":/res/fonts/Inconsolata-Regular.ttf"},
   {types::Font::RobotoFlex_Regular, ":/res/fonts/RobotoFlex-Regular.ttf"}};

void Initialize()
{
   LoadFonts();
   LoadTextures();
}

void Shutdown() {}

std::shared_ptr<boost::gil::rgba8_image_t>
LoadImageResource(const std::string& urlString)
{
   util::TextureAtlas& textureAtlas = util::TextureAtlas::Instance();
   return textureAtlas.CacheTexture(urlString, urlString);
}

std::vector<std::shared_ptr<boost::gil::rgba8_image_t>>
LoadImageResources(const std::vector<std::string>& urlStrings)
{
   std::mutex                                              m {};
   std::vector<std::shared_ptr<boost::gil::rgba8_image_t>> images {};

   std::for_each(std::execution::par_unseq,
                 urlStrings.begin(),
                 urlStrings.end(),
                 [&](auto& urlString)
                 {
                    auto image = LoadImageResource(urlString);

                    if (image != nullptr)
                    {
                       std::unique_lock lock {m};
                       images.emplace_back(std::move(image));
                    }
                 });

   if (!images.empty())
   {
      BuildAtlas();
   }

   return images;
}

static void LoadFonts()
{
   auto& fontManager = FontManager::Instance();

   for (auto& fontName : fontNames_)
   {
      fontManager.LoadApplicationFont(fontName.first, fontName.second);
   }

   fontManager.InitializeFonts();
}

static void LoadTextures()
{
   util::TextureAtlas& textureAtlas = util::TextureAtlas::Instance();

   for (auto imageTexture : types::ImageTextureIterator())
   {
      textureAtlas.RegisterTexture(GetTextureName(imageTexture),
                                   GetTexturePath(imageTexture));
   }

   for (auto lineTexture : types::LineTextureIterator())
   {
      textureAtlas.RegisterTexture(GetTextureName(lineTexture),
                                   GetTexturePath(lineTexture));
   }

   BuildAtlas();
}

void BuildAtlas()
{
   util::TextureAtlas& textureAtlas = util::TextureAtlas::Instance();
   textureAtlas.BuildAtlas(atlasWidth, atlasHeight);
}

} // namespace ResourceManager
} // namespace manager
} // namespace qt
} // namespace scwx
