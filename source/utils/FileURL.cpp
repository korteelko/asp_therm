#include "FileURL.h"

#include <assert.h>


namespace file_utils {
SetupURL::SetupURL(url_t url_type, const std::string &root)
  : url_type(url_type), root(root) {}


FileURL::FileURL(url_t url_type, const std::string &path)
  : status_(STATUS_DEFAULT), url_type_(url_type), absolute_path_(path) {}

FileURL::FileURL()
  : status_(STATUS_NOT), url_type_(url_t::empty) {}

bool FileURL::IsInvalidPath() const {
  return !is_status_aval(status_);
}

void FileURL::SetError(merror_t error) {
  error_.SetError(error);
  status_ = STATUS_HAVE_ERROR;
}

void FileURL::SetError(merror_t error, const std::string &msg) {
  error_.SetError(error, msg);
  status_ = STATUS_HAVE_ERROR;
}

void FileURL::LogError() {
  error_.LogIt();
}


/* FileURLCreator */
FileURLRoot::FileURLRoot(const SetupURL &setup)
  : status_(STATUS_DEFAULT), setup_(setup) {
  if (setup_.GetURLType() == url_t::fs_path)
    check_fs_root();
}

bool FileURLRoot::IsInitialized() {
  return is_status_ok(status_);
}

FileURL FileURLRoot::CreateFileURL(const std::string &relative_path) {
  switch (setup_.GetURLType()) {
    case url_t::fs_path:
      return set_fs_path(relative_path);
    case url_t::empty:
      break;
  }
  return FileURL();
}

void FileURLRoot::check_fs_root() {
  // если строка пути к руту не пустая, проверить
  //   чем она закансивается
  /* todo:  */
  if (!setup_.root.empty())
    if (!ends_with(setup_.root, "/"))
      setup_.root += "/";
  status_ = STATUS_OK;
}

FileURL FileURLRoot::set_fs_path(const std::string &relative_path) {
  return FileURL(setup_.GetURLType(), setup_.GetFullPrefix() + relative_path);
}
}  // namespace file_urls
