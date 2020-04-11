#include "FileURL.h"

#include "gtest/gtest.h"

#include <iostream>
#include <filesystem>
#include <numeric>

#include <assert.h>


namespace fs = std::filesystem;
using namespace file_utils;

static fs::path cwd;
static fs::path data_root = "../../../data/gases";
static fs::path exists_file = "methane.xml";
static fs::path not_exists_file = "notmethane.xml";

/** \brief Тестинг класса FileURL для путей в файловой
  *   системе(локальных файлов) */
class FileURLFilesystemTest: public ::testing::Test {
protected:
  FileURLFilesystemTest()
    : fc(SetupURL(url_t::fs_path, data_root.string())) {
      EXPECT_TRUE(fc.IsInitialized());
    }

  ~FileURLFilesystemTest() {}

protected:
  FileURLRoot fc;
};

//

/** \brief Проверка существования файла */
TEST_F(FileURLFilesystemTest, FilesExistsTest) {
  FileURL ex = fc.CreateFileURL(exists_file.string());
  std::cerr << "Проверяем файл: " << fs::exists(ex.GetURL());
  EXPECT_TRUE(fs::exists(ex.GetURL()));
  FileURL not_ex = fc.CreateFileURL(not_exists_file.string());
  EXPECT_FALSE(fs::exists(not_ex.GetURL()));
  not_ex.SetError(ERROR_FILE_EXISTS_ST, "файл не сущесттвует");
  EXPECT_TRUE(not_ex.IsInvalidPath());
}

int main(int argc, char **argv) {
  cwd = fs::path(argv[0]).parent_path();
  fs::current_path(cwd);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
