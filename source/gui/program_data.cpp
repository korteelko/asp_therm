#include "program_data.h"

#include "target_sys.h"

#include <QDir>

#include <algorithm>

#include <assert.h>
#include <string.h>

#if defined(_OS_NIX)
#  include <dirent.h>
#  include <sys/types.h>
#  include <unistd.h>
#elif defined(_OS_WIN)
#  include <windows.h>
#endif  // _OS

#if defined(_OS_WIN)
#  define PATH_DELIMETER '\\'
const std::string data_path  = "\\..\\..\\..\\data\\";
const std::string gases_dir = "gases\\";
#elif defined(_OS_NIX)
#  define PATH_DELIMETER '/'
const std::string data_path  = "/../../../data/";
const std::string gases_dir = "gases/";
#endif  // _OS


namespace {
std::string get_file_name(const std::string &full_path) {
  size_t dot_pos = full_path.rfind('.'),
         del_pos = full_path.rfind(PATH_DELIMETER);
  if (dot_pos == std::string::npos)
    dot_pos = full_path.size();
  if (del_pos == std::string::npos)
    del_pos = 0;
  return full_path.substr(del_pos + 1, dot_pos - del_pos - 1);
}
}  // anonymous namespace

ProgramData::ProgramData()
  : error_status_(ERR_SUCCESS_T) {
  init_gas_files();
  init_gas_names();
}

ProgramData &ProgramData::Instance() {
  static ProgramData pd;
  return pd;
}

ERROR_TYPE ProgramData::GetError() {
  return error_status_;
}

bool ProgramData::ends_with(const char *src, const char *ending) {
  int tmp_end = strlen(ending);
  int len_dif = strlen(src) - tmp_end;
  if (len_dif < 0)
    return false;
  while (tmp_end-- > 0)
    if (src[len_dif + tmp_end] != ending[tmp_end])
      return false;
  return true;
}

void ProgramData::init_gas_files() {
  QString cwd = QDir::currentPath();
  error_status_ = read_gases_dir(cwd.toStdString() + data_path + gases_dir);
}

void ProgramData::init_gas_names() {
  if (!gas_names_.empty())
    gas_names_.clear();
  for (const auto &x : gas_files_)
    gas_names_.push_back(get_file_name(x));
}

int ProgramData::read_gases_dir(const std::string &dirname) {
  if (!gas_files_.empty())
    gas_files_.clear();
#if defined(_OS_NIX)
  DIR *dirptr;
  struct dirent *drn;
  if ((dirptr = opendir(dirname.c_str())) == NULL) {
    set_error_message(ERR_INIT_T, "cannot open dir with gases xml");
    return ERR_INIT_T;
  }
  while ((drn = readdir(dirptr)) != NULL)
    if (ends_with(drn->d_name, ".xml"))
      gas_files_.push_back(dirname + drn->d_name);
  closedir(dirptr);
  return ERR_SUCCESS_T;
#elif defined(_OS_WIN)
  assert(0);
#endif  // _OS
}

const std::vector<std::string> &ProgramData::GetGasesList() {
  return gas_names_;
}
