#ifndef UTILS__INODE_H
#define UTILS__INODE_H

#include "common.h"

#include <string>
#include <vector>

#include <stdint.h>


// typedef int32_t node_id;

/** \brief Интерфейс узла для составления параметров шаблона
  *   классов JSONReader и XMLReader
  * \note По идее обвязка над структурой(ами) которые нужно
  *   инициализировать парсером */
template <class node_t>
class INodeInitializer {
public:
  /** \brief вектор имён дочерних элементов */
  typedef std::vector<std::string> inodes_vec;

public:
  INodeInitializer() {}
  virtual ~INodeInitializer() {}

  // node_id GetId() const { return id_; }
  std::string GetName() const { return name_; }

  virtual bool IsLeafNode() const = 0;
  virtual void SetParentData(INodeInitializer *parent) = 0;
  virtual void SetSubnodesNames(inodes_vec *subnodes) = 0;

protected:
  /** \brief уникальный идентификатор ноды */
  // node_id id_;
  /** \brief собственное имя ноды */
  std::string name_;
  /** \brief вектор имён поднод(подузлов)
    * \note по ним из класса парсера их можно инициализировать */
  inodes_vec subnodes_;
  /** \brief собственные данные ноды */
  node_t data_;
  bool have_subnodes_;
};

#endif  // !UTILS__INODE_H
