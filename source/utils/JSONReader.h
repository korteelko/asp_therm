#ifndef UTILS__JSONREADER_H
#define UTILS__JSONREADER_H

#include "common.h"
#include "ErrorWrap.h"
// рано)
// #include "FileURL.h"
#include "Logging.h"

#include <rapidjson/document.h>

#include <memory>
#include <fstream>
#include <functional>
#include <vector>


namespace rj = rapidjson;

/* кстати, можно для xml реализовать также.
 * алсо, node_t можно реализовать под оба ридера */
// такс, все необходимые для JSONReader операции
//   реализуем здесь(на основе этой структуры запилим
//   также для файла конфигурации, стилей и базовой графики)
struct node_example {
  /* todo: чёт для конфигурации графики подход газовых смесей не оч -
   *   там был более-менее стандартизированный формат xml файла,
   *   здесь надо придумать что-то обтекаемое */
  // bool IsLeafNode(const std::string &node);
  // bool IsLeafNode(const rj::Value &node);
public:
  std::string name;
};
// для тиестов
// inline bool node_example::IsLeafNode(const std::string &node) { return "control"; }


/** \brief шаблон класса дерева json файла, стандартная обёртка
  *   над инициализируемой нодой
  * \note такс, для базы это структура 'control_base' или
  *   обёртка над ней: control_base_creator, а скорее всего GUIBase */
template <class node_t>
class json_node {
  /** \brief умный указатель на имплементацию json_node */
  typedef std::unique_ptr<json_node<node_t>> json_node_ptr;
  /** \brief вектор указателей на дочерние элементы */
  typedef std::vector<json_node_ptr> childs_vec;

public:
  json_node(rj::Value *src)
    : source_(src) {
      // инициализировать шаблон-параметр node_data и
      //   дочерние элементы(в глубину обойти)
      initData();
      child_it = childs.begin();
    }
  /** \brief Добавить ноду к списку инициализированных(если можно)
    *   вытащив её параметры, в зависимости от типа ноды засунуть их
    *   в dst
    * \param src исходный узел json файла
    * \param dst ссылка на структуру, которую необходимо заполнить
    *   данными из src
    * \note Здесь надо вытащить имя(тип) ноды и прокинуть его
    *   в класс node_t, чтобы тонкости реализации выполнял он
    *   Ну и пока не ясно что делать с иерархичностью */
  // void AppendNode(const rj::Value &src) { assert(0); }
  /** \brief добавить дочерний узел
    * \note */
  // void AddChild(const rj::Value &src) { assert(0); }
  /** \brief  */
  //bool SetNextChild(const rj::Value &src) {
  //  return node_data.InitData(src);
  //}
  /** \brief Проверить наличие дочерних элементов ноды */
  // bool IsLeafNode() const { return node_data.IsLeafNode(); }

  /** \brief Проверить наличие дочерних элементов ноды */
  json_node<node_t> *NextChild() {
    return (child_it != childs.end()) ?
        child_it++->get() : nullptr;
  }
  /** \brief Инициализировать иерархию
    * \note тут такое, я пока неопределился id ноды тащить
    *   из файла конфигурации или выдавать здесь, так как без
    *   id не связываются структуры стилей из отдельного файла
    *   с базовой иерархией из основного файла */
  void SetHierarchy() {
    for (auto x: childs) {
      x->node_data.SetParentId(node_data.GetId());
      x->SetHierarchy();
    }
  }
  /** \brief Получить json исходник */
  rj::Value *GetSource() const {
    return source_;
  }
  /** \brief */
  merror_t GetError() const {
    return error_.GetErrorCode();
  }

private:
  /** \brief Инициализировать данные ноды */
  void initData() {
    if (source_) {
      error_ = node_data.InitData(source_);
      if (!error_.GetErrorCode())
        error_ = initChilds();
    }
  }
  /** \brief Получить список имён подузлов узла
    *   с именем 'curr_node' */
  // void ne_nujna();
  /** \brief Инициализировать дочерние элементы узла */
  void initChilds() {
    // забрать названия подузлов,
    //   хотя сейчас всё сделано так что все подузлы однотипны
    //   и собраны в один контейнер, дальновиднее подготовить
    //   вектор входных данных
    std::vector<std::string> subtrees;
    // в зависимости от типа узла название составляющих отличается
    //   получим их названия
    node_data.SetContent(&subtrees);
    // если вложенные поддеревья есть - обойдём
    for (const auto &st_name: subtrees) {
      rj::Document::MemberIterator it = source_->FindMember(st_name.c_str());
      if (it != source_->MemberEnd()) {
        rj::Value &chs = *source_[st_name];
        if (chs.IsArray()) {
          // если это массив(а мы его и ищем),
          //   инициализируем подузлы()
          for (rj::SizeType i = 0; i < chs.Size(); ++i)
            childs.emplace_back(json_node_ptr(chs[i]));
        } else {
          error_.SetError(ERROR_JSON_FORMAT_ST, "Не соответствующий формат "
              "json файла для узла компонентов: " + st_name);
        }
      }
    }
  }

private:
  ErrorWrap error_;
  /** \brief ссылка на представление ноды в rj */
  rj::Value *source_;

public:
  /** \brief ссылка на родительский элемент(unused) */
  // json_node *parent;
  /** \brief дочерние элементы */
  childs_vec childs;
  /** \brief итератор на обход дочерних элементов */
  typename childs_vec::iterator child_it;
  /** \brief инициализируемая структура */
  node_t node_data;
};


/** \brief Класс парсинга json файлов
  * \note По идее здесь главным должен быть реализоывн метод
  *   позволяет вытащить весь скелет структур с++ привязанных
  *   json нодам(кстати)
  *   Для случая нашего в рут нодах храняться id родительских элементов */
template <class node_t>
class JSONReader {
public:
  /** \brief callback функция инициализации элементов
    * \note не нравится мне это. Лучше определить в
    *   классе-параметре шаблона */
  //typedef std::function<
  //    std::vector<std::string> &(const rj::Value &, node_t *)> callback_f;

public:
  JSONReader(const file_utils::FileURL &source)
    : status_(STATUS_DEFAULT), source_(source), file_mem_(nullptr) {
    if (!source_.IsInvalidPath()) {
      init_file_mem();
      if (!error_.GetErrorCode()) {
        // распарсить json файл
        document_.Parse(file_mem_);
        // проверить рут
        if (document_.IsObject()) {
          root_node_ = std::unique_ptr<json_node<node_t>>(
              new json_node<node_t>(document_));
          tree_traversal(root_node_.get());
          if (!error_.GetErrorCode())
            status_ = STATUS_OK;
        } else {
          error_.SetError(ERROR_JSON_PARSE_ST, "ошибка инициализации "
              "корневого элемента json файла " + source_.GetURL());
        }
      }
    }
    if (error_.GetErrorCode()) {
      status_ = STATUS_HAVE_ERROR;
      error_.LogIt();
    }
  }

  ~JSONReader() {
    if (file_mem_)
      delete[] file_mem_;
  }

  // typedef std::vector<std::string> TreePath;
  /* что-то я хз */
  // rj::Value &ValueByPath(const TreePath &) {assert(0);}

private:
  /** \brief обход дерева json объектов,
    * \note вынесено в отдельный метод потому-что можно
    *   держать лополнительное состояние, например,
    *   глубину обхода(см питоновский скрипт в asp_therm) */
  void tree_traversal( //rj::Value *src,
      json_node<node_t> *jnode) {
    json_node<node_t> *child;
    // rj::Value *child_src;
    // обход дочерних элементов
    while ((child = jnode->NextChild()) != nullptr) {
      tree_traversal(child);
    }
  }

  /** \brief считать файл в память */
  void init_file_mem() {
    size_t len = 0;
    std::ifstream fstr(source_.GetURL());
    if (fstr) {
      fstr.seekg (0, fstr.end);
      len = fstr.tellg();
      fstr.seekg (0, fstr.beg);
      if (len > 0) {
        file_mem_ = new char[len];
        fstr.read(file_mem_, len);
        if (!fstr) {
          error_.SetError(ERROR_FILE_IN_ST, "File read error for: " +
              source_.GetURL());
          Logging::Append(io_loglvl::err_logs, "ошибка чтения json файла: "
              "из " + std::to_string(len) + " байт считано " +
              std::to_string(fstr.gcount()));
        }
      } else {
        // ошибка файла
        error_.SetError(ERROR_FILE_IN_ST, "File length error for: " +
            source_.GetURL());
      }
    } else {
      // ошибка открытия файла
      error_.SetError(ERROR_FILE_EXISTS_ST, "File open error for: " +
          source_.GetURL());
    }
  }

private:
  ErrorWrap error_;
  mstatus_t status_;
  /** \brief адрес файла */
  file_utils::FileURL source_;
  /** \brief буффер памяти файла */
  char *file_mem_;
  /** \brief основной json объект */
  rj::Document document_;
  /** \brief корень json дерева
    * \note а в этом сетапе он наверное и не обязателен */
  std::unique_ptr<json_node<node_t>> root_node_;
};

#endif  // UTILS__JSONREADER_H

