#ifndef UTILS__THREADWRAP_H
#define UTILS__THREADWRAP_H

#include <mutex>


template <class T>
class MutexTemplate  {
public:
  MutexTemplate() = default;
  MutexTemplate(const MutexTemplate &) {}
    // /*mutex_(T())*/ { (void)m; }

  T &Get() { return mutex_; }

  void lock() { mutex_.lock(); }
  void unlock() { mutex_.unlock(); }
  bool try_lock() { return mutex_.try_lock(); }

public:
  T mutex_;
};

/** \brief Обёртка над обычным мьютексом
  * \note Можно сделать шаблоном если понадобиться */
using Mutex = MutexTemplate<std::mutex>;

/** \brief Обёртка над обычным рекурсивным мьютексом
  * \note Можно сделать шаблоном если понадобиться */
using RecursiveMutex = MutexTemplate<std::recursive_mutex>;

#endif  // !UTILS__THREADWRAP_H
