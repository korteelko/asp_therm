/**
 * asp_therm - implementation of real gas equations of state
 * ===================================================================
 * * program_state *
 *   В файле описан класс состояния программы, инкапсулирующий
 * взаимодействие всех основных модулей программы
 * ===================================================================
 *
 * Copyright (c) 2020-2021 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef _CORE__SERVICE__PROGRAM_STATE_H_
#define _CORE__SERVICE__PROGRAM_STATE_H_

#include "asp_db/db_connection_manager.h"
#include "asp_utils/Common.h"
#include "asp_utils/ErrorWrap.h"
#include "asp_utils/FileURL.h"
#include "asp_utils/ThreadWrap.h"
#include "atherm_db_tables.h"
#include "calculation_setup.h"
#include "configuration_by_file.h"
#include "models_configurations.h"
#include "xml_reader.h"

#include <atomic>
#include <memory>
#include <optional>

/** \brief Макро на определение режима отладки */
#if defined(DATABASE_TEST)
#undef IS_DEBUG_MODE
#define IS_DEBUG_MODE true
#else
#undef IS_DEBUG_MODE
#define IS_DEBUG_MODE (ProgramState::Instance().IsDebugMode())
#endif

/**
 * \brief Класс конфигурации состояния программы
 * */
class ProgramConfiguration : public BaseObject {
 public:
  ProgramConfiguration();
  ProgramConfiguration(const std::string& config_filename);

  merror_t ResetConfigFile(const std::string& new_config_filename);

 private:
  /** 
   * \brief Установить значения по умолчанию
   *   для возможных параметров
   * */
  void setDefault();
  /**
   * \brief Считать и инициализировать конфигурацию программы
   * */
  void initProgramConfig();
  /**
   * \brief Считать и инициализировать конфигурацию
   *   коннекта к базе данных
   * */
  void initDatabaseConfig();
  /**
   * \brief Считать и инициализировать конфигурацию модели
   * */
  // model_str initModelStr();

 public:
  /**
   * \brief Текущий файл конфигурации
   * */
  std::string config_filename;
  /**
   * \brief Конфигурация программы
   * */
  program_configuration configuration;
  /**
   * \brief Параметры коннекта к БД
   * */
  std::optional<asp_db::db_parameters> db_parameters_conf{std::nullopt};
  /**
   * \brief По-сути - декоратор над объектом чтения xml(или других форматов)
   *   файлов для конфигурации программы
   * \note На тестинг инстанцируем шаблон заранее
   * */
  /* todo: remove instance, add template parameter */
  std::unique_ptr<ConfigurationByFile<XMLReader>> config_by_file;
  /**
   * \brief Чтение файла завершилось успешной загрузкой
   *   конфигуции программы
   * */
  bool is_initialized;
};


/**
 * \brief Класс состояния программы:
 *   конфигурация программы, информация о моделях, подключение к БД,
 *   конфигурации расчётов(области, используемые модели etc)
 */
class ProgramState : public BaseObject {
 public:
  /** \brief Синглетончик инст */
  static ProgramState& Instance();

  /* Инициализация */
  /**
   * \brief Инициализировать рабочую директорию приложения
   */
  void SetProgramDirs(const file_utils::FileURLRoot& work_dir,
                      const file_utils::FileURLRoot& calc_dir);
  /**
   * \brief Загрузить или перезагрузить конфигурацию программы
   * */
  merror_t ReloadConfiguration(const std::string& config_file);
  /**
   * \brief Обновить конфигурацию БД
   * */
  void UpdateDatabaseStructure();
  /**
   * \brief Конфигурация из файла была загружена
   * \return true да, false нет
   * */
  bool IsInitialized() const;

  /* Конфигурация программы */
  /**
   * \brief Приложение запущено в режиме отладки
   * \return true да, false нет
   * */
  bool IsDebugMode() const;
  /**
   * \brief Приложение работает без подключения к бд
   * \return true да, false нет
   */
  bool IsDryRunDBConn() const;
  const program_configuration& GetConfiguration() const;
  const calculation_configuration& GetCalcConfiguration() const;
  const std::optional<asp_db::db_parameters> &GetDatabaseConfiguration() const;

  /* Расчёт */
  /**
   * \brief Добавить сетап расчёта
   * \param filepath Путь к сетапу расчёта относительно calc_dir_
   * \return id расчётных параметров или -1 в случае ошибки
   * */
  int AddCalculationSetup(const std::string& filepath);
  /**
   * \brief Запустить расчёт
   * \param num Номер сетапа расчёта
   * */
  void RunCalculationSetup(int num);
  /**
   * \brief Удалить сетап расчёта
   * \param num Номер сетапа расчёта
   * */
  void RemoveCalculationSetup(int num);

 private:
  ProgramState();

 private:
  Mutex state_mutex;
  Mutex calc_mutex;
  /**
   * \brief Объект инициализации путей, привязан к корневой
   *   директории программы, т.е. от неё отсчитываем пути
   *   к файлам конфигурации, данным, ресурсам и т.п.
   * */
  std::shared_ptr<file_utils::FileURLRoot> work_dir_ = nullptr;
  /**
   * \brief Объект иниализации путей связанных с данными расчётов
   * */
  std::shared_ptr<file_utils::FileURLRoot> calc_dir_ = nullptr;
  /* todo: они не связаны между собой, можно распараллелить
   *   also, всё-таки речь идёт не о контейнере, а о полноценном объекте */
  std::atomic<int> calc_key;
  /**
   * \brief Набор данных для проведения расчётов
   * */
  Calculations calc_setups_;
  /**
   * \brief Конфигурация программы - модели, бд, опции
   * */
  ProgramConfiguration program_config_;
  /**
   * \brief Менеджер подключения к БД
   * */
  asp_db::DBConnectionManager db_manager_;
};


#endif  // !_CORE__SERVICE__PROGRAM_STATE_H_
