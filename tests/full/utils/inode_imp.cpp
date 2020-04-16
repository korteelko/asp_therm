#include "inode_imp.h"


json_test_node::json_test_node() {
  name_ = "";
}

void json_test_node::SetParentData(json_test_node &parent) {
  // there is dynamic_cast<>
  parent_name = parent.GetName();
}

merror_t json_test_node::InitData(rj::Value *src, const std::string &_name) {
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

std::string json_test_node::GetParameter(const std::string &name) {
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

merror_t json_test_node::GetFirst(first *f) {
  if (name_ == "first" && f) {
    f->f = GetParameter("f");
    f->s = GetParameter("s");
    f->ff = GetParameter("ff");
    f->t = std::stof(GetParameter("t"));
    return ERROR_SUCCESS_T;
  }
  return ERROR_GENERAL_T;
}

merror_t json_test_node::GetSecond(second *s) {
  if (name_ == "second" && s) {
    s->f = GetParameter("f");
    s->s = std::atoi(GetParameter("s").c_str());
    s->t = std::atoi(GetParameter("t").c_str());
    return ERROR_SUCCESS_T;
  }
  return ERROR_GENERAL_T;
}

void json_test_node::SetSubnodesNames(inodes_vec *s) {
  s->clear();
  s->insert(s->end(), subnodes_.cbegin(), subnodes_.cend());
}

std::string json_test_node::get_root_name() {
  // return node_t_list[NODE_T_ROOT];
  return "test";
}

/** \brief проверить наличие подузлов
  * \note просто посмотрим тип этого узла,
  *   а для случая придумаю что-нибудь */
void json_test_node::set_subnodes() {
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
