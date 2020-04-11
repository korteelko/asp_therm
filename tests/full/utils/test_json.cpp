#include "common.h"
#include "file_structs.h"
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

/** \brief тестовая структура-параметр шаблона XMLReader */
struct test_node {
  // static const std::array<std::string, 3> node_t_list;
  struct first {
    std::string f, s, ff;
    float t;
  };
  struct second {
    std::string f;
    int s, t;
  };
public:
  // node_type config_node_type;
  rj::Value *source;
  std::string parent_name;
  std::string name;
  std::vector<std::string> subnodes;
  bool have_subnodes;

public:
  test_node()
    : name("") {}

  std::string GetName() const { return name; }

  bool IsLeafNode() const { return have_subnodes; }

  void SetParentData(test_node *parent) {
    parent_name = parent->GetName();
  }

  merror_t InitData(rj::Value *src) {
    if (!src)
      return ERROR_INIT_NULLP_ST;
    source = src;
    merror_t error = ERROR_SUCCESS_T;
    if (source->HasMember("type")) {
      rj::Value &tmp_nd = source->operator[]("type");
      name = tmp_nd.GetString();
      set_subnodes();
    }
    if (name.empty()) {
      error = ERROR_JSON_FORMAT_ST;
    }
    return error;
  }

  std::string GetParameter(const std::string &name) {
    std::string value = "";
    rj::Value::MemberIterator it = source->FindMember("data");
    if (it != source->MemberEnd()) {
      rj::Value &data = it->value;
      if (data.HasMember(name.c_str())) {
        rj::Value &par = data[name.c_str()];
        if (par.IsString()) {
          value = par.GetString();
        } else if (par.IsInt()) {
          value = std::to_string(par.GetInt());
        } else if (par.IsDouble()) {
          value = std::to_string(par.GetDouble());
        }
      }
    }
    return value;
  }

  merror_t GetFirst(first *f) {
    if (name == "first" && f) {
      f->f = GetParameter("f");
      f->s = GetParameter("s");
      f->ff = GetParameter("ff");
      f->t = std::stof(GetParameter("t"));
      return ERROR_SUCCESS_T;
    }
    return ERROR_GENERAL_T;
  }

  merror_t GetSecond(second *s) {
    if (name == "second" && s) {
      s->f = GetParameter("f");
      s->s = std::atoi(GetParameter("s").c_str());
      s->t = std::atoi(GetParameter("t").c_str());
      return ERROR_SUCCESS_T;
    }
    return ERROR_GENERAL_T;
  }

  /** \brief Записать имена узлов, являющихся
    *   контейнерами других объектов */
  void SetContent(std::vector<std::string> *s) {
    s->clear();
    s->insert(s->end(), subnodes.cbegin(), subnodes.cend());
  }

  static std::string get_root_name() {
    // return node_t_list[NODE_T_ROOT];
    return "test";
  }
  static node_type get_node_type(std::string type) {
    (void) type;
    //for (uint32_t i = 0; i < node_t_list.size(); ++i)
    //  if (node_t_list[i] == type)
    //    return i;
    return NODE_T_UNDEFINED;
  }

private:
  /** \brief проверить наличие подузлов
    * \note просто посмотрим тип этого узла,
    *   а для случая придумаю что-нибудь */
  void set_subnodes() {
    subnodes.clear();
    if (name == "test") {
      have_subnodes = true;
      subnodes.push_back("data");
    } if (name == "first") {
      have_subnodes = true;
    } if (name == "second") {
      have_subnodes = true;
    } else {
      have_subnodes = false;
    }
  }
};


/** \brief класс тестов парсера json файла - JSONReader'a */
class JSONReaderTest: public ::testing::Test {
protected:
  JSONReaderTest()
   : file_c(file_utils::SetupURL(file_utils::url_t::fs_path, testdir.string())) {
    EXPECT_TRUE(file_c.IsInitialized());
    auto path =  file_c.CreateFileURL(tf.string());
    EXPECT_TRUE(fs::exists(path.GetURL()));
    JSONReader_ = std::unique_ptr<JSONReader<test_node>>(
        JSONReader<test_node>::Init(&path));
    EXPECT_TRUE(JSONReader_ != nullptr);
  }

  ~JSONReaderTest() override {}

protected:
  file_utils::FileURLRoot file_c;
  std::unique_ptr<JSONReader<test_node>> JSONReader_;
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
  EXPECT_EQ(trim_str(res), "116.2");
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
