#include "Common.h"
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
    auto path = file_c.CreateFileURL(tf.string());
    EXPECT_TRUE(fs::exists(path.GetURL()));
    initJR();
    initJRF();
  }

  void initJR() {
    auto path = file_c.CreateFileURL(tf.string());
    JSONReader_ = std::unique_ptr<JSONReaderSample<json_test_node<>>>(
        JSONReaderSample<json_test_node<>>::Init(&path));
    if (JSONReader_ != nullptr)
      JSONReader_->InitData();
    else
      EXPECT_TRUE(false);
  }

  void initJRF() {
    auto path = file_c.CreateFileURL(tf.string());
    node_factory.factory_num = 8800;
    JSONReaderF_ = std::unique_ptr<JSONReaderSample<json_test_node<>, json_test_factory>>(
        JSONReaderSample<json_test_node<>, json_test_factory>::Init(&path, &node_factory));
    if (JSONReaderF_ != nullptr)
      JSONReaderF_->InitData();
    else
      EXPECT_TRUE(false);
  }

  ~JSONReaderTest() override {}

protected:
  file_utils::FileURLRoot file_c;
  json_test_factory node_factory;
  /* однажды typedef'ы победят уродские имена в 2 строки */
  std::unique_ptr<JSONReaderSample<json_test_node<>>> JSONReader_;
  std::unique_ptr<JSONReaderSample<json_test_node<>,
      json_test_factory>> JSONReaderF_;
};

/** \brief Тест инициализации JsonReader */
TEST_F(JSONReaderTest, ReadFile) {
  ASSERT_TRUE(JSONReader_ != nullptr);
  merror_t error = JSONReader_->GetErrorCode();
  /* проверить инициализацию */
  if (error) {
    std::cerr << "\tJSONReader cause error: " << hex2str(error) << std::endl;
    std::cerr << JSONReader_->GetFileName() << std::endl;
    ASSERT_EQ(error, ERROR_SUCCESS_T);
  }
}
/** \brief Тест инициализации JsonReaderF */
TEST_F(JSONReaderTest, ReadFileF) {
  ASSERT_TRUE(JSONReaderF_ != nullptr);
  merror_t error = JSONReaderF_->GetErrorCode();
  /* проверить инициализацию */
  if (error) {
    std::cerr << "\tJSONReader with node factory cause error: "
        << hex2str(error) << std::endl;
    std::cerr << JSONReaderF_->GetFileName() << std::endl;
    ASSERT_EQ(error, ERROR_SUCCESS_T);
  }
}

/** \brief Макро на вытягивание стринги */
#define macro_string(x, path, res, val) do {\
  EXPECT_EQ(x->GetValueByPath(path, res), ERROR_SUCCESS_T);\
  EXPECT_EQ(*res, val);\
  } while(0)
/** \brief Макро на вытягивание значения с плавающей точкой */
#define macro_float(x, path, res, val) do {\
  EXPECT_EQ(x->GetValueByPath(path, res), ERROR_SUCCESS_T);\
  EXPECT_NEAR(std::stod(*res), val, 0.00001);\
  } while(0)

/** \brief Тест вытягивания параметров(наследие XMLReader) */
TEST_F(JSONReaderTest, ValueByPath) {
  std::vector<std::string> path_emp;
  std::string res = "";
  macro_string(JSONReader_, path_emp, &res, "");
  macro_string(JSONReaderF_, path_emp, &res, "");
  /* вытянуть обычный параметр */
  std::vector<std::string> path_f = {"first", "s"};
  macro_string(JSONReader_, path_f, &res, "sdsa");
  macro_string(JSONReaderF_, path_f, &res, "sdsa");
  /* вытянуть параметр плавающей точки */
  path_f[1] = "t";
  macro_float(JSONReader_, path_f, &res, 116.2);
  macro_float(JSONReaderF_, path_f, &res, 116.2);
  path_f[1] = "ff";
  macro_string(JSONReader_, path_f, &res, "");
  macro_string(JSONReaderF_, path_f, &res, "");
  /* попытаться вытянуть то чего там нет */
  path_f[0] = "wrong name";
  EXPECT_NE(JSONReader_->GetValueByPath(path_f, &res), ERROR_SUCCESS_T);
  EXPECT_NE(JSONReaderF_->GetValueByPath(path_f, &res), ERROR_SUCCESS_T);
}

TEST_F(JSONReaderTest, Factory) {
  std::vector<std::string> path_f = {"first"};
  auto x = JSONReaderF_->GetNodeByPath(path_f);
  ASSERT_TRUE(x != nullptr);
  EXPECT_EQ(x->data.factory_num, node_factory.factory_num);
}

int main(int argc, char **argv) {
  cwd = fs::path(argv[0]).parent_path();
  fs::current_path(cwd);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
