# asp\_therm

**ToDo**

- [ ] разбить модели на области
- [ ] добавить БД
- [ ] добавить JSON конфиг
- [ ] прикрутить модуль динамики(по мере необходимости)

В проекте реализован матаппарат расчёта параметров газовых смесей. Используемые уравнений состояния реального газа, или, для краткости, *модели*, были выбраны руководствуясь главным образом успехом применениях при описании газовых смесей близких по составу к природному газу.

### Подключение PostgreSQL

*Для управления БД [PostgreSQL](https://www.postgresql.org) можно использовать [pgAdmin4](https://www.pgadmin.org)*.   
**Пример**  
*Имя БД - 'africae', имя пользователя(user или role) - 'jorge', пароль пользователя 'my\_pass'*.   
Первым делом необходимо переключиться на пользователя postgres и запустим cli субд:   
`$ sudo -u postgres psql`   
После запуска cli создадим базу данных и пользователя, предоставив ему все привилегии над этой БД:    
`=# create database africae;`  
`=# create user jorge with encrypted password 'my_pass';`   
`=# grant all privileges on database africae to jorge;`   
Посмотреть состояние субд можно командой `\l`. Полный help отобразит `\?`.   
Для завершения сессии введите команду '`exit;`' или `\q`.     
После соответствующих изменений секция базы данных в файле конфигурации configuration.xml будет выглядеть примерно так(см. configuration\_template.xml):    
<code>\<group name="database">    
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

