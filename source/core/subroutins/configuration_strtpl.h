/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef _CORE__SUBROUTINS__CONFIGURATION_STRTPL_H_
#define _CORE__SUBROUTINS__CONFIGURATION_STRTPL_H_

/* Выпишем все возможные значения строковых параметров,
     представленных в файлах конфигурации и входных параметров
   Зачем:
     Если другие файлы тоже буду парситься програмно, или потребуется
       состояние скипать в текстовое... интересная идея для дампов состояния */

/* TEMPLATES */
/* параметры конфигурации */
#define STRTPL_CONFIG_DEBUG_MODE "debug_mode"
#define STRTPL_CONFIG_RK_SOAVE_MOD "rk_soave_mod"
#define STRTPL_CONFIG_PR_BINARYCOEFS "pr_binary_coefs"
#define STRTPL_CONFIG_INCLUDE_ISO_20765 "include_iso_20765"
#define STRTPL_CONFIG_LOG_LEVEL "log_level"
#define STRTPL_CONFIG_LOG_FILE "log_file"
#define STRTPL_CONFIG_DATABASE "database"

/* параметры конфигурации базы данных */
#define STRTPL_CONFIG_DB_DRY_RUN "dry_run"
#define STRTPL_CONFIG_DB_CLIENT "client"
#define STRTPL_CONFIG_DB_NAME "name"
#define STRTPL_CONFIG_DB_USERNAME "username"
#define STRTPL_CONFIG_DB_PASSWORD "password"
#define STRTPL_CONFIG_DB_HOST "host"
#define STRTPL_CONFIG_DB_PORT "port"

/* VALUES */
/* bool */
#define STRTPL_BOOL_TRUE "true"
#define STRTPL_BOOL_FALSE "false"

/* log_level */
#define STRTPL_LOG_LEVEL_NO_LOGS "no_logs"
#define STRTPL_LOG_LEVEL_ERR "err"
#define STRTPL_LOG_LEVEL_WARN "warn"
#define STRTPL_LOG_LEVEL_DEBUG "debug"

/* clients */
#define STRTPL_DB_CLIENT_NOONE "noone"
#define STRTPL_DB_CLIENT_POSTGRESQL "postgresql"

#endif  // !_CORE__SUBROUTINS__CONFIGURATION_STRTPL_H_
