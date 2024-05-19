## Тестовое задание
Команды для сборки 
```
cmake -B build -G "generator-name"
cmake --build build
```
Где "generator-name" имя генератора для создания файлов системы сборки
Например: "Unix Makefiles", "Visual Studio 17 2022"

Для проверки тестов создан скрипт. Команда для запуска:
```
./script.sh "executable-name"
```
Где "executable-name" имя созданного исполняемого файла