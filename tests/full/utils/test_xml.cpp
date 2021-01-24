#include "Common.h"
#include "xml_reader.h"

#include "gtest/gtest.h"

#include <array>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace fs = std::filesystem;

static const std::array<std::string, 3> node_t_list =
    std::array<std::string, 3>{"test_root", "group", "parameter"};

/**
 * \brief тестовая структура-параметр шаблона XMLReader
 * */
struct test_node {
  // static const std::array<std::string, 3> node_t_list;

 public:
  node_type config_node_type;
  std::string name, value;

 public:
  test_node(node_type itype, std::string name)
      : config_node_type(itype), name(name), value("") {}
  test_node(node_type itype, std::string name, std::string value)
      : config_node_type(itype), name(name), value(value) {}

  static std::string get_root_name() { return node_t_list[NODE_T_ROOT]; }
  static node_type get_node_type(std::string type) {
    for (uint32_t i = 0; i < node_t_list.size(); ++i)
      if (node_t_list[i] == type)
        return i;
    return NODE_T_UNDEFINED;
  }
};

/** \brief класс тестов парсера xml файла - XMLReader'a */
class XMLReaderTest : public ::testing::Test {
 protected:
  XMLReaderTest() {
    cwd = fs::current_path();
    setup_example_data();
    XMLReader_ = std::unique_ptr<XMLReader<test_node>>(
        XMLReader<test_node>::Init((cwd / testdir / tf).string()));
    EXPECT_TRUE(XMLReader_ != nullptr);
  }

  ~XMLReaderTest() override {}

 protected:
  /**
   * \brief Создать папку для данных и сгенерировать файлы
   * */
  void setup_example_data() {
    fs::path d = cwd / testdir;
    fs::path xf = d / tf;
    if (not is_exists(d.string()))
      fs::create_directory(d);
    std::ofstream fx(xf);
    fx << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
          "<test_root name=\"test\">\n"
          "  <group name=\"first\">\n"
          "    <parameter name=\"f\"> sda </parameter>\n"
          "    <parameter name=\"s\"> sdsa </parameter>\n"
          "    <parameter name=\"t\"> 116.2 </parameter>\n"
          "    <parameter name=\"ff\">  </parameter>\n"
          "  </group>\n"
          "  <group name=\"second\">\n"
          "    <parameter name=\"f\"> asd </parameter>\n"
          "    <parameter name=\"s\"> 32 </parameter>\n"
          "    <parameter name=\"t\"> 12 </parameter>\n"
          "  </group>\n"
          "</test_root>\n";
    fx.close();
  }

 protected:
  std::unique_ptr<XMLReader<test_node>> XMLReader_;
  fs::path cwd;
  fs::path testdir = "xmldir";
  fs::path tf = "test_xml.xml";
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

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
