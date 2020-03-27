# asp\_therm


В проекте реализован матаппарат расчёта теплофизических параметров реальных газов и их смесей. Используемые уравнения состояния реального газа, или, для краткости, *модели*, были выбраны руководствуясь главным образом успехом применения их для описания газовых смесей близких по составу к природному газу.   
Ссылки на используемую техническую литературу по физике газов прописаны контекстно в коде, и в разделе [Литература](#literature).

### <a name="literature"></a> Литература

- Рид Р., Праусниц Дж., Шервуд Т. "Свойства газов и жидкостей: Справочное пособие". Л.:Химия, 1982.
- Павловский В.А. "Введение в термодинамику реальных газов". _Тут данные издания_.
- Брусиловский А.И. "Фазовые превращения при разработке месторождений нефти и газа". М.:Грааль, 2002.
- Казунин Д.В. _Информация по монографии_
- ГОСТ 30319 -- 2015.
  - ГОСТ 30319.1 -- 2015. Межгосударственный стандарт. Газ природный. Методы расчёта физических свойств. Общие положения. М.: Стандартинформ, 2015.
  - ГОСТ 30319.3 -- 2015. Межгосударственный стандарт. Газ природный. Методы расчёта физических свойств. Вычисление свойств на основе данных о компонентном составе. М.: Стандартинформ, 2015.
- ISO 20765-1. International standard. Natural gas -- Calculation of thermodynamic properties. Part 1: Gas phase properties for transmission and distribution applications. ISO 2005.    
*Алсо, вторая часть этого стандарта тоже интересна*

## Программное обеспечение
### Сборка
1. Копировать репозиторий и подмодули:
`git clone --recursive https://github.com/sinkarl/asp_therm`
2. Установить [зависимости](#dependencies)
3. Создать файл конфигурации, например из файла *configuration_template.xml*
4. Запустить cmake

Если не использовать ide, стандартный набор команд сетапа проекта с cmake примерно такой:  
`cd asp_therm`  
`mkdir build`  
`cd build`  
`cmake ..`  
`make`


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
<code>\<group name="database"></code>    
<code>&nbsp;&nbsp;\<parameter name="dry\_run"> false \</parameter></code>      
<code>&nbsp;&nbsp;\<parameter name="client"> postgresql \</parameter></code>    
<code>&nbsp;&nbsp;\<parameter name="name"> africae \</parameter></code>    
<code>&nbsp;&nbsp;\<parameter name="username"> jorge \</parameter></code>     
<code>&nbsp;&nbsp;\<parameter name="password"> my\_pass \</parameter></code>    
<code>&nbsp;&nbsp;\<parameter name="host"> 127.0.0.1 \</parameter></code>    
<code>&nbsp;&nbsp;\<parameter name="port"> 5432 \</parameter></code>     
<code>\</group></code>

### Подмодули:

- [**pugixml**](https://github.com/zeux/pugixml) - парсинг xml-файлов
- [**rapidjson**](https://github.com/Tencent/rapidjson) - парсинг json-файлов

### <a name="dependencies"></a> Зависимости:

- [libpqxx](http://pqxx.org/development/libpqxx/) - С++ клиент для PostgreSQL
- *опционально* [gtest](https://github.com/google/googletest) - C++ тест фрэймворк от Google

------
### Прочее
**Тесты**   
В папке `tests` есть несколько подпроектов тестов, существование которых долгое время игнорировалось, но всё же культура ведения разработки требует их полноценного функционирования, а не безмолвного присутствия.     
В директории `tests/full` находится проект гуглотестов. Проект специализирован CMakeLists.txt файлом.


**ToDo**

- [ ] разбить модели на области
- [ ] добавить БД
- [ ] добавить JSON конфиг
- [ ] прикрутить модуль динамики(по мере необходимости)
- [ ] gtest модулем добавить
- [ ] данные компонентов забить в базу, и, собственно, добавить .cpp модуль на чтение этих параметров из неё.