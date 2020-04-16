#include "common.h"
#include "file_structs.h"
#include "XMLReader.h"

#include "gtest/gtest.h"

#include <array>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>


namespace fs = std::filesystem;

static fs::path cwd;
static fs::path testdir = "../../../tests/full/utils/data/";
static fs::path tf = "test_xml.xml";

/* в тестовом файле прописано:
<?xml version="1.0" encoding="UTF-8"?>
<test_root name="test">
  <group name="first">
    <parameter name="f"> sda </parameter>
    <parameter name="s"> sdsa </parameter>
    <parameter name="t"> 116.2 </parameter>
    <parameter name="ff">  </parameter>
  </group>
  <group name="second">
    <parameter name="f"> asd </parameter>
    <parameter name="s"> 32 </parameter>
    <parameter name="t"> 12 </parameter>
  </group>
</test_root>
*/
static const std::array<std::string, 3> node_t_list =
   std::array<std::string, 3> {"test_root", "group", "parameter"};

/** \brief тестовая структура-параметр шаблона XMLReader */
struct test_node {
  // static const std::array<std::string, 3> node_t_list;

public:
  node_type config_node_type;
  std::string name,
              value;

public:
  test_node(node_type itype, std::string name)
    : config_node_type(itype), name(name), value("") {}
  test_node(node_type itype, std::string name, std::string value)
    : config_node_type(itype), name(name), value(value) {}

  static std::string get_root_name() {
    return node_t_list[NODE_T_ROOT];
  }
  static node_type get_node_type(std::string type) {
    for (uint32_t i = 0; i < node_t_list.size(); ++i)
      if (node_t_list[i] == type)
        return i;
    return NODE_T_UNDEFINED;
  }
};


/** \brief класс тестов парсера xml файла - XMLReader'a */
class XMLReaderTest: public ::testing::Test {
protected:
  XMLReaderTest() {
    XMLReader_ = std::unique_ptr<XMLReader<test_node>>(
        XMLReader<test_node>::Init((cwd / testdir / tf).string()));
    EXPECT_TRUE(XMLReader_ != nullptr);
  }

  ~XMLReaderTest() override {}

protected:
  std::unique_ptr<XMLReader<test_node>> XMLReader_;
};

TEST_F(XMLReaderTest, ReadFileClassic) {
  ASSERT_TRUE(XMLReader_ != nullptr);
  std::vector<std::string> xml_path_emp;
  std::string res = "";
  /* проверить инициализацию */
  ASSERT_EQ(XMLReader_->GetErrorCode(), ERROR_SUCCESS_T);
  EXPECT_EQ(XMLReader_->GetRootName(), "test_root");
  /* вытянуть рут */
  EXPECT_EQ(XMLReader_->GetValueByPath(xml_path_emp, &res), ERROR_SUCCESS_T);
  EXPECT_EQ(res, "");
  /* вытянуть обычный параметр */
  std::vector<std::string> xml_path_f = {"first", "s"};
  EXPECT_EQ(XMLReader_->GetValueByPath(xml_path_f, &res), ERROR_SUCCESS_T);
  EXPECT_EQ(trim_str(res), "sdsa");
  xml_path_f[1] = "t";
  EXPECT_EQ(XMLReader_->GetValueByPath(xml_path_f, &res), ERROR_SUCCESS_T);
  EXPECT_EQ(trim_str(res), "116.2");
  xml_path_f[1] = "ff";
  EXPECT_EQ(XMLReader_->GetValueByPath(xml_path_f, &res), ERROR_SUCCESS_T);
  EXPECT_EQ(trim_str(res), "");
  /* попытаться вытянуть то чего там нет */
  xml_path_f[0] = "wrong name";
  EXPECT_NE(XMLReader_->GetValueByPath(xml_path_f, &res), ERROR_SUCCESS_T);
}

TEST_F(XMLReaderTest, DISABLED_ReadFileNew) {
  ASSERT_TRUE(false);
}

int main(int argc, char **argv) {
  cwd = fs::path(argv[0]).parent_path();
  fs::current_path(cwd);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
