# asp_therm

**ToDo**

- [ ] разбить модели на области
- [ ] добавить БД
- [ ] добавить JSON конфиг
- [ ] прикрутить модуль динамики(по мере необходимости)

В проекте реализован матаппарат расчёта параметров газовых смесей. Используемые уравнений состояния реального газа, или, для краткости, *модели*, были выбраны руководствуясь главным образом успехом применениях при описании газовых смесей близких по составу к природному газу.

### Подключение PostgreSQL

*Для управления БД [PostgreSQL](https://www.postgresql.org) можно использовать [pgAdmin4](https://www.pgadmin.org)*.  
**Пример настройки**   
После установки сервера postgre создадим пользователя *jorge* с паролем *my_pass* и базу данных *africae*.   
Переключимся на пользователя postgres и запустим cli субд   
`$ sudo -u postgres psql`   
Создадим базу данных *africae* и пользователя *jorge* с соответствующими привилегиями   
`=# create database africae;`  
`=# create user jorge with encrypted password 'my_pass';`   
`=# grant all privileges on database africae to jorge;`   
Посмотреть результат можно командой `\l`. Полный help можно посмотреть командой `\?`.   
Для завершения сессии введите команду '`exit;`' или `\q`.     
Соответсвенно, после соответствующих изменений секция базы данных в файле конфигурации configuration.xml будет выглядеть примерно так:    
<code>
\<group name="database">    
&nbsp;&nbsp;\<parameter name="dry\_run"> false \</parameter>    
&nbsp;&nbsp;\<parameter name="client"> postgresql \</parameter>    
&nbsp;&nbsp;\<parameter name="name"> africae \</parameter>    
&nbsp;&nbsp;\<parameter name="username"> jorge \</parameter>    
&nbsp;&nbsp;\<parameter name="password"> my\_pass \</parameter>     
&nbsp;&nbsp;\<parameter name="host"> 127.0.0.1 \</parameter>    
&nbsp;&nbsp;\<parameter name="port"> 5432 \</parameter>    
\</group> 
 </code>



### ПО
#### Подмодули:

- [**pugixml**](https://github.com/zeux/pugixml) - парсинг xml-файлов
- [**rapidjson**](https://github.com/Tencent/rapidjson) - парсинг json-файлов

#### Зависимости:

- [libpqxx](http://pqxx.org/development/libpqxx/) - С++ клиент для PostgreSQL

