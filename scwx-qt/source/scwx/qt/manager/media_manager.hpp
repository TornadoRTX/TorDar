#pragma once

#include <scwx/qt/types/media_types.hpp>

#include <memory>

namespace scwx::qt::manager
{

class MediaManager
{
public:
   explicit MediaManager();
   ~MediaManager();

   MediaManager(const MediaManager&)            = delete;
   MediaManager& operator=(const MediaManager&) = delete;

   MediaManager(MediaManager&&) noexcept;
   MediaManager& operator=(MediaManager&&) noexcept;

   void Play(types::AudioFile media);
   void Play(const std::string& mediaPath);
   void Stop();

   static std::shared_ptr<MediaManager> Instance();

private:
   class Impl;
   std::unique_ptr<Impl> p;
};

} // namespace scwx::qt::manager
