#ifndef TESTS__UTILS__INODE_IMP_H
#define TESTS__UTILS__INODE_IMP_H

#include "ErrorWrap.h"
#include "INode.h"
#include "JSONReader.h"
//#include "XMLReader.h"

#include <string>
#include <vector>


/** \brief пример структуры которую нужно инициализировать */
struct example_node {
  std::string name;
  int factory_num;
};

/** \brief тестовая структура-параметр шаблонов Парсеров */
template<class T = example_node>
class json_test_node: public INodeInitializer {
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
  rj::Value *source;
  std::string name, parent_name;
  example_node data;
  bool have_subnodes_;

public:
  json_test_node() {
    name_ = "";
  }
  /** \brief Имя верхнего узла документа */
  static std::string get_root_name() {
    return "test";
  }
  /** \brief Инициализировать данные структуры */
  merror_t InitData(rj::Value *src, const std::string &_name) {
    if (!src)
      return ERROR_INIT_NULLP_ST;
    name = _name;
    source = src;
    merror_t error = ERROR_SUCCESS_T;
    /* type is magic word for json test files */
    if (source->HasMember("type")) {
      rj::Value &tmp_nd = source->operator[]("type");
      name_ = tmp_nd.GetString();
      // инициализировать данные инициализируемой структуры
      data.name = name_;
      set_subnodes();
    }
    if (name_.empty()) {
      error = ERROR_JSON_FORMAT_ST;
    }
    return error;
  }

  void SetParentData(json_test_node &parent) {
    parent_name = parent.GetName();
  }

  // override
  bool IsLeafNode() const override {
    return have_subnodes_;
  }
  /** \brief получить поле узла структуры */
  void SetSubnodesNames(inodes_vec *s) override {
    s->clear();
    s->insert(s->end(), subnodes_.cbegin(), subnodes_.cend());
  }
  /** \brief получить поле узла структуры */
  std::string GetParameter(const std::string &name) override {
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
  /* jfl */
  /** \brief инициализировать параметры тестовой структуры first */
  merror_t GetFirst(first *f) {
    if (name_ == "first" && f) {
      f->f = GetParameter("f");
      f->s = GetParameter("s");
      f->ff = GetParameter("ff");
      f->t = std::stof(GetParameter("t"));
      return ERROR_SUCCESS_T;
    }
    return ERROR_GENERAL_T;
  }
  /** \brief инициализировать параметры тестовой структуры second */
  merror_t GetSecond(second *s) {
    if (name_ == "second" && s) {
      s->f = GetParameter("f");
      s->s = std::atoi(GetParameter("s").c_str());
      s->t = std::atoi(GetParameter("t").c_str());
      return ERROR_SUCCESS_T;
    }
    return ERROR_GENERAL_T;
  }

private:
  /** \brief проверить наличие подузлов
    * \note просто посмотрим тип этого узла,
    *   а для случая придумаю что-нибудь */
  void set_subnodes() {
    subnodes_.clear();
    if (name_ == "test") {
      have_subnodes_ = true;
      subnodes_.push_back("data");
    } if (name_ == "first") {
      have_subnodes_ = true;
    } if (name_ == "second") {
      have_subnodes_ = true;
    } else {
      have_subnodes_ = false;
    }
  }
};

/** \brief Фабрика создания json_test_node */
class json_test_factory {
public:
  json_test_node<> *GetNodeInitializer();

public:
  int factory_num;
};

#endif  // !TESTS__UTILS__INODE_IMP_H
