#ifndef TESTS__UTILS__INODE_IMP_H
#define TESTS__UTILS__INODE_IMP_H

#include "common.h"
#include "ErrorWrap.h"
#include "INode.h"
#include "JSONReader.h"
#include "XMLReader.h"

#include <string>
#include <vector>

/** \brief тестовая структура-параметр шаблонов Парсеров */
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
  std::string parent_name;

public:
  json_test_node();

  static std::string get_root_name();

  // override
  void SetParentData(INodeInitializer *parent) override;
  void SetSubnodesNames(inodes_vec *s) override;

  /** \brief Инициализировать данные структуры */
  merror_t InitData(rj::Value *src);
  /** \brief получить поле узла структуры */
  std::string GetParameter(const std::string &name);
  /** \brief инициализировать параметры тестовой структуры first */
  merror_t GetFirst(first *f);
  /** \brief инициализировать параметры тестовой структуры second */
  merror_t GetSecond(second *s);

private:
  /** \brief проверить наличие подузлов
    * \note просто посмотрим тип этого узла,
    *   а для случая придумаю что-нибудь */
  void set_subnodes();
};

#endif  // !TESTS__UTILS__INODE_IMP_H
