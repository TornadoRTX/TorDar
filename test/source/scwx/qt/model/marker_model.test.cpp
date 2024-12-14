#include <scwx/qt/model/marker_model.hpp>
#include <scwx/qt/manager/marker_manager.hpp>
#include <scwx/qt/main/application.hpp>
#include <scwx/qt/util/color.hpp>

#include <filesystem>
#include <fstream>
#include <QObject>

#include <condition_variable>
#include <gtest/gtest.h>

namespace scwx
{
namespace qt
{
namespace model
{

static const std::string EMPTY_MARKERS_FILE =
   std::string(SCWX_TEST_DATA_DIR) + "/json/markers/markers-empty.json";
static const std::string TEMP_MARKERS_FILE =
   std::string(SCWX_TEST_DATA_DIR) + "/json/markers/markers-temp.json";
static const std::string ONE_MARKERS_FILE =
   std::string(SCWX_TEST_DATA_DIR) + "/json/markers/markers-one.json";
static const std::string FIVE_MARKERS_FILE =
   std::string(SCWX_TEST_DATA_DIR) + "/json/markers/markers-five.json";
static const std::string PART1_MARKER_FILE =
   std::string(SCWX_TEST_DATA_DIR) + "/json/markers/markers-part1.json";

static std::mutex              initializedMutex {};
static std::condition_variable initializedCond {};
static bool                    initialized;

static const boost::gil::rgba8_pixel_t defaultIconColor =
   util::color::ToRgba8PixelT("#ffff0000");
static const std::string defaultIconName = "images/location-marker";

void CompareFiles(const std::string& file1, const std::string& file2)
{
   std::ifstream     ifs1 {file1};
   std::stringstream buffer1;
   buffer1 << ifs1.rdbuf();

   std::ifstream     ifs2 {file2};
   std::stringstream buffer2;
   buffer2 << ifs2.rdbuf();

   EXPECT_EQ(buffer1.str(), buffer2.str());
}

void CopyFile(const std::string& from, const std::string& to)
{
   std::filesystem::copy_file(from, to);
   CompareFiles(from, to);
}

using TestFunction = void(std::shared_ptr<manager::MarkerManager>,
                          MarkerModel&);

void RunTest(const std::string& filename, TestFunction testFunction)
{
   {
      main::Application::ResetInitilization();
      MarkerModel                             model = MarkerModel();
      std::shared_ptr<manager::MarkerManager> manager =
         manager::MarkerManager::Instance();

      manager->set_marker_settings_path(TEMP_MARKERS_FILE);

      initialized = false;
      QObject::connect(manager.get(),
                       &manager::MarkerManager::MarkersInitialized,
                       []()
                       {
                          std::unique_lock lock(initializedMutex);
                          initialized = true;
                          initializedCond.notify_all();
                       });

      main::Application::FinishInitialization();

      std::unique_lock lock(initializedMutex);
      while (!initialized)
      {
         initializedCond.wait(lock);
      }
      // This is not jank
      model.HandleMarkersInitialized(manager->marker_count());

      testFunction(manager, model);
   }

   EXPECT_EQ(std::filesystem::exists(TEMP_MARKERS_FILE), true);

   CompareFiles(TEMP_MARKERS_FILE, filename);
}

TEST(MarkerModelTest, CreateJson)
{
   // Verify file doesn't exist prior to test start
   EXPECT_EQ(std::filesystem::exists(TEMP_MARKERS_FILE), false);

   RunTest(EMPTY_MARKERS_FILE,
           [](std::shared_ptr<manager::MarkerManager>, MarkerModel&) {});

   std::filesystem::remove(TEMP_MARKERS_FILE);
   EXPECT_EQ(std::filesystem::exists(TEMP_MARKERS_FILE), false);
}

TEST(MarkerModelTest, LoadEmpty)
{
   CopyFile(EMPTY_MARKERS_FILE, TEMP_MARKERS_FILE);

   RunTest(EMPTY_MARKERS_FILE,
           [](std::shared_ptr<manager::MarkerManager>, MarkerModel&) {});

   std::filesystem::remove(TEMP_MARKERS_FILE);
   EXPECT_EQ(std::filesystem::exists(TEMP_MARKERS_FILE), false);
}

TEST(MarkerModelTest, AddRemove)
{
   CopyFile(EMPTY_MARKERS_FILE, TEMP_MARKERS_FILE);

   RunTest(ONE_MARKERS_FILE,
           [](std::shared_ptr<manager::MarkerManager> manager, MarkerModel&)
           {
              manager->add_marker(types::MarkerInfo(
                 "Null", 0, 0, defaultIconName, defaultIconColor));
           });
   RunTest(
      EMPTY_MARKERS_FILE,
      [](std::shared_ptr<manager::MarkerManager> manager, MarkerModel& model)
      {
         std::optional<types::MarkerId> id = model.getId(0);
         EXPECT_TRUE(id);
         if (id)
         {
            manager->remove_marker(*id);
         }
      });

   std::filesystem::remove(TEMP_MARKERS_FILE);
   EXPECT_EQ(std::filesystem::exists(TEMP_MARKERS_FILE), false);
}

TEST(MarkerModelTest, AddFive)
{
   CopyFile(EMPTY_MARKERS_FILE, TEMP_MARKERS_FILE);

   RunTest(FIVE_MARKERS_FILE,
           [](std::shared_ptr<manager::MarkerManager> manager, MarkerModel&)
           {
              manager->add_marker(types::MarkerInfo(
                 "Null", 0, 0, defaultIconName, defaultIconColor));
              manager->add_marker(types::MarkerInfo(
                 "North", 90, 0, defaultIconName, defaultIconColor));
              manager->add_marker(types::MarkerInfo(
                 "South", -90, 0, defaultIconName, defaultIconColor));
              manager->add_marker(types::MarkerInfo(
                 "East", 0, 90, defaultIconName, defaultIconColor));
              manager->add_marker(types::MarkerInfo(
                 "West", 0, -90, defaultIconName, defaultIconColor));
           });

   std::filesystem::remove(TEMP_MARKERS_FILE);
   EXPECT_EQ(std::filesystem::exists(TEMP_MARKERS_FILE), false);
}

TEST(MarkerModelTest, AddFour)
{
   CopyFile(ONE_MARKERS_FILE, TEMP_MARKERS_FILE);

   RunTest(FIVE_MARKERS_FILE,
           [](std::shared_ptr<manager::MarkerManager> manager, MarkerModel&)
           {
              manager->add_marker(types::MarkerInfo(
                 "North", 90, 0, defaultIconName, defaultIconColor));
              manager->add_marker(types::MarkerInfo(
                 "South", -90, 0, defaultIconName, defaultIconColor));
              manager->add_marker(types::MarkerInfo(
                 "East", 0, 90, defaultIconName, defaultIconColor));
              manager->add_marker(types::MarkerInfo(
                 "West", 0, -90, defaultIconName, defaultIconColor));
           });

   std::filesystem::remove(TEMP_MARKERS_FILE);
   EXPECT_EQ(std::filesystem::exists(TEMP_MARKERS_FILE), false);
}

TEST(MarkerModelTest, RemoveFive)
{
   CopyFile(FIVE_MARKERS_FILE, TEMP_MARKERS_FILE);

   RunTest(
      EMPTY_MARKERS_FILE,
      [](std::shared_ptr<manager::MarkerManager> manager, MarkerModel& model)
      {
         std::optional<types::MarkerId> id;
         id = model.getId(4);
         EXPECT_TRUE(id);
         manager->remove_marker(*id);

         id = model.getId(3);
         EXPECT_TRUE(id);
         manager->remove_marker(*id);

         id = model.getId(2);
         EXPECT_TRUE(id);
         manager->remove_marker(*id);

         id = model.getId(1);
         EXPECT_TRUE(id);
         manager->remove_marker(*id);

         id = model.getId(0);
         EXPECT_TRUE(id);
         manager->remove_marker(*id);
      });

   std::filesystem::remove(TEMP_MARKERS_FILE);
   EXPECT_EQ(std::filesystem::exists(TEMP_MARKERS_FILE), false);
}

TEST(MarkerModelTest, RemoveFour)
{
   CopyFile(FIVE_MARKERS_FILE, TEMP_MARKERS_FILE);

   RunTest(
      ONE_MARKERS_FILE,
      [](std::shared_ptr<manager::MarkerManager> manager, MarkerModel& model)
      {
         std::optional<types::MarkerId> id;
         id = model.getId(4);
         EXPECT_TRUE(id);
         manager->remove_marker(*id);

         id = model.getId(3);
         EXPECT_TRUE(id);
         manager->remove_marker(*id);

         id = model.getId(2);
         EXPECT_TRUE(id);
         manager->remove_marker(*id);

         id = model.getId(1);
         EXPECT_TRUE(id);
         manager->remove_marker(*id);
      });

   std::filesystem::remove(TEMP_MARKERS_FILE);
   EXPECT_EQ(std::filesystem::exists(TEMP_MARKERS_FILE), false);
}

TEST(MarkerModelTest, UpdateFromPart1)
{
   CopyFile(PART1_MARKER_FILE, TEMP_MARKERS_FILE);

   RunTest(ONE_MARKERS_FILE,
           [](std::shared_ptr<manager::MarkerManager>, MarkerModel&) {});

   std::filesystem::remove(TEMP_MARKERS_FILE);
   EXPECT_EQ(std::filesystem::exists(TEMP_MARKERS_FILE), false);
}

} // namespace model
} // namespace qt
} // namespace scwx
