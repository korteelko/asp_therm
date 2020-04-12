#ifndef UTILS__JSONREADER_H
#define UTILS__JSONREADER_H

#include "common.h"
#include "ErrorWrap.h"
#include "FileURL.h"
#include "Logging.h"
#include "INode.h"

#include "rapidjson/document.h"
#include "rapidjson/error/en.h"

#include <memory>
#include <fstream>
#include <functional>
#include <vector>


namespace rj = rapidjson;

/** \brief шаблон класса дерева json файла, стандартная обёртка
  *   над инициализируемой нодой
  * \note такс, для базы это структура 'control_base' или
  *   обёртка над ней: control_base_creator, а скорее всего GUIBase */
//template <class Initializer, typename = std::enable_if_t<
//    std::is_base_of<INodeInitializer, Initializer>::value>>
template <class Initializer, typename = std::enable_if_t<
    std::is_base_of<INodeInitializer, Initializer>::value>>
class json_node {
  /** \brief умный указатель на имплементацию json_node */
  typedef std::unique_ptr<json_node<Initializer>> json_node_ptr;
  /** \brief вектор указателей на дочерние элементы */
  typedef std::vector<json_node_ptr> childs_vec;

public:
  /** \brief Добавить ноду к списку инициализированных(если можно)
    *   вытащив её параметры, в зависимости от типа ноды засунуть их
    *   в dst
    * \param src исходный узел json файла
    * \param dst ссылка на структуру, которую необходимо заполнить
    *   данными из src
    * \note Здесь надо вытащить имя(тип) ноды и прокинуть его
    *   в класс node_t, чтобы тонкости реализации выполнял он
    *   Ну и пока не ясно что делать с иерархичностью */
  json_node(rj::Value *src)
    : value_(src) {
      // инициализировать шаблон-параметр node_data и
      //   дочерние элементы(в глубину обойти)
      initData();
      child_it = childs.begin();
    }
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
  json_node<Initializer> *NextChild() {
    return (child_it != childs.end()) ?
        child_it++->get() : nullptr;
  }
  /** \brief Поиск по дочерним элементам */
  json_node<Initializer> *ChildByName(const std::string &name) const {
    json_node<Initializer> *child = nullptr;
    if (!node_data.IsLeafNode()) {
      for (const json_node_ptr &ch: childs) {
        if (ch->node_data.GetName() == name) {
          child = ch.get();
          break;
        }
      }
    }
    return child;
  }
  /** \brief Инициализировать иерархичные данные
    * \note тут такое, я пока неопределился id ноды тащить
    *   из файла конфигурации или выдавать здесь, так как без
    *   id не связываются структуры стилей из отдельного файла
    *   с базовой иерархией из основного файла */
  void SetParentData() {
    for (auto &x: childs) {
      x->node_data.SetParentData(&node_data);
      x->SetParentData();
    }
  }
  /** \brief Получить строковое представление параметра
    * \note Так-то актуально только для параметров */
  std::string GetParameter(const std::string &name) {
    return node_data.GetParameter(name);
  }
  /** \brief Получить json исходник */
  rj::Value *GetSource() const {
    return value_;
  }
  /** \brief Получить код ошибки */
  merror_t GetError() const {
    return error_.GetErrorCode();
  }
  /** \brief Узел - массив */
  // bool IsArray() const { return is_array_; }

private:
  /** \brief Инициализировать данные ноды */
  void initData() {
    if (value_) {
      /* инициализировать */
      error_.SetError(node_data.InitData(value_));
      if (!error_.GetErrorCode())
        initChilds();
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
    // в зависимости от типа узла название составляющих(подузлов)
    //   отличается получим их названия
    node_data.SetSubnodesNames(&subtrees);
    // если вложенные поддеревья есть - обойдём
    for (const auto &st_name: subtrees) {
      //rj::Document::MemberIterator it = value_->FindMember(st_name.c_str());
      //if (it != value_->MemberEnd()) {
      if (value_->HasMember(st_name.c_str())) {
        rj::Value &chs = value_->operator[](st_name.c_str());
        for (rj::Value::MemberIterator it = chs.MemberBegin(); it < chs.MemberEnd(); ++it) {
          if (it->value.IsObject())
            childs.emplace_back(json_node_ptr(new json_node<Initializer>(&it->value)));
        }
      }
    }
  }

private:
  ErrorWrap error_;
  /** \brief ссылка на представление ноды в rj */
  rj::Value *value_;
  /** \brief узел массив */
  // bool is_array_;

public:
  /** \brief ссылка на родительский элемент(unused) */
  // json_node *parent;
  /** \brief дочерние элементы */
  childs_vec childs;
  /** \brief итератор на обход дочерних элементов */
  typename childs_vec::iterator child_it;
  // такс, все необходимые для JSONReader операции
  //   реализуем здесь
  /** \brief инициализируемая структура */
  Initializer node_data;
};


/** \brief Класс парсинга json файлов
  * \note По идее здесь главным должен быть реализоывн метод
  *   позволяет вытащить весь скелет структур с++ привязанных
  *   json нодам(кстати)
  *   Для случая нашего в рут нодах храняться id родительских элементов */
template <class Initializer>
class JSONReader {
public:
  /** \brief callback функция инициализации элементов
    * \note не нравится мне это. Лучше определить в
    *   классе-параметре шаблона */
  //typedef std::function<
  //    std::vector<std::string> &(const rj::Value &, node_t *)> callback_f;

public:
  static JSONReader<Initializer> *Init(file_utils::FileURL *source) {
    JSONReader<Initializer> *reader = nullptr;
    if (source) {
      if (is_exist(source->GetURL())) {
        reader = new JSONReader<Initializer>(*source);
      } else {
        source->SetError(ERROR_FILE_EXISTS_ST, "File '" +
            source->GetURL() + "' doesn't exists");
        source->LogError();
      }
    } else {
      Logging::Append(ERROR_INIT_NULLP_ST, "Get nullptr into JSONReader "
          "Init method");
    }
    return reader;
  }

  ~JSONReader() {
    if (file_mem_)
      delete[] file_mem_;
  }

  static std::string GetFilenameExtension() {
    return ".json";
  }

  std::string GetFileName() {
    return source_.GetURL();
  }

  merror_t GetErrorCode() const {
    return error_.GetErrorCode();
  }
  /** \brief Получить параметр по переданному пути
    * \note Функция обобщённого обхода
    * \warning outstr придёт с пробелами, если они есть в xml,
    *   алсо путь принимается без рут ноды */
  merror_t GetValueByPath(const std::vector<std::string> &json_path,
      std::string *outstr) {
    if (!root_node_)
      return ERROR_GENERAL_T;
    /* todo: добавить const квалификатор */
    json_node<Initializer> *tmp_node = root_node_.get();
    std::string param = "";
    if (!json_path.empty()) {
      param = json_path.back();
      for (auto i = json_path.begin(); i != json_path.end() - 1; ++i) {
        if (tmp_node)
          tmp_node = tmp_node->ChildByName(*i);
        else
          break;
      }
      if (!tmp_node) {
        return FILE_LAST_OBJECT;
      }
    }
    *outstr = tmp_node->GetParameter(param);
    return ERROR_SUCCESS_T;
  }

  // typedef std::vector<std::string> TreePath;
  /* что-то я хз */
  // rj::Value &ValueByPath(const TreePath &) {assert(0);}

private:
  JSONReader(const file_utils::FileURL &source)
    : status_(STATUS_DEFAULT), source_(source), file_mem_(nullptr) {
    init_file_mem();
    if (!error_.GetErrorCode()) {
      // распарсить json файл
      document_.Parse(file_mem_);
      // проверить рут
      if (document_.IsObject()) {
        if (document_.HasParseError()) {
          error_.SetError(ERROR_JSON_FORMAT_ST,
              std::string("RapidJSON parse error: ") +
              std::string(rj::GetParseError_En(document_.GetParseError())));
        }
        root_node_ = std::unique_ptr<json_node<Initializer>>(
            new json_node<Initializer>(&document_));
        tree_traversal(root_node_.get());
        root_node_->SetParentData();
        if (!error_.GetErrorCode())
          status_ = STATUS_OK;
      } else {
        error_.SetError(ERROR_JSON_PARSE_ST, "ошибка инициализации "
            "корневого элемента json файла " + source_.GetURL());
      }
    }
    if (error_.GetErrorCode()) {
      status_ = STATUS_HAVE_ERROR;
      error_.LogIt();
    }
  }
  /** \brief обход дерева json объектов,
    * \note вынесено в отдельный метод потому-что можно
    *   держать лополнительное состояние, например,
    *   глубину обхода(см питоновский скрипт в asp_therm) */
  void tree_traversal(json_node<Initializer> *jnode) {
    json_node<Initializer> *child;
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
  std::unique_ptr<json_node<Initializer>> root_node_;
};

#endif  // !UTILS__JSONREADER_H
