#include "common.h"
#include "file_structs.h"
#include "inode_imp.h"
#include "JSONReader.h"

#include "gtest/gtest.h"

#include <filesystem>
#include <memory>
#include <string>
#include <vector>


namespace fs = std::filesystem;

static fs::path cwd;
static fs::path testdir = "../../../tests/full/utils/data/";
static fs::path tf = "test_json.json";

/* в тестовом файле прописано:
 {
  "type": "test",
  "data": {
    "d1": {
      "type": "first",
      "data": {
        "f": "sda",
        "s": "sdsa",
        "t": 116.2,
        "ff": ""
      }
    },
    "d2": {
      "type": "second",
      "data": {
        "f": "asd",
        "s": 32,
        "t": 12
      }
    }
  }
}
*/


/** \brief класс тестов парсера json файла - JSONReader'a */
class JSONReaderTest: public ::testing::Test {
protected:
  JSONReaderTest()
   : file_c(file_utils::SetupURL(file_utils::url_t::fs_path, testdir.string())) {
    EXPECT_TRUE(file_c.IsInitialized());
    auto path =  file_c.CreateFileURL(tf.string());
    EXPECT_TRUE(fs::exists(path.GetURL()));
    JSONReader_ = std::unique_ptr<JSONReaderSample<json_test_node>>(
        JSONReaderSample<json_test_node>::Init(&path));
    if (JSONReader_ != nullptr)
      JSONReader_->InitData();
    else
      EXPECT_TRUE(false);
  }

  ~JSONReaderTest() override {}

protected:
  file_utils::FileURLRoot file_c;
  std::unique_ptr<JSONReaderSample<json_test_node>> JSONReader_;
};

TEST_F(JSONReaderTest, ReadFile) {
  ASSERT_TRUE(JSONReader_ != nullptr);
  merror_t error = JSONReader_->GetErrorCode();
  /* проверить инициализацию */
  if (error) {
    std::cerr << "\t" << hex2str(error) << std::endl;
    std::cerr << JSONReader_->GetFileName() << std::endl;
    ASSERT_EQ(error, ERROR_SUCCESS_T);
  }
}

TEST_F(JSONReaderTest, ValueByPath) {
  std::vector<std::string> path_emp;
  std::string res = "";
  /* вытянуть рут */
  EXPECT_EQ(JSONReader_->GetValueByPath(path_emp, &res), ERROR_SUCCESS_T);
  EXPECT_EQ(res, "");
  /* вытянуть обычный параметр */
  std::vector<std::string> path_f = {"first", "s"};
  EXPECT_EQ(JSONReader_->GetValueByPath(path_f, &res), ERROR_SUCCESS_T);
  EXPECT_EQ(trim_str(res), "sdsa");
  path_f[1] = "t";
  EXPECT_EQ(JSONReader_->GetValueByPath(path_f, &res), ERROR_SUCCESS_T);
  EXPECT_NEAR(std::stod(res), 116.2, 0.00001);
  path_f[1] = "ff";
  EXPECT_EQ(JSONReader_->GetValueByPath(path_f, &res), ERROR_SUCCESS_T);
  EXPECT_EQ(trim_str(res), "");
  /* попытаться вытянуть то чего там нет */
  path_f[0] = "wrong name";
  EXPECT_NE(JSONReader_->GetValueByPath(path_f, &res), ERROR_SUCCESS_T);

}

int main(int argc, char **argv) {
  cwd = fs::path(argv[0]).parent_path();
  fs::current_path(cwd);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
