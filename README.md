# lapki-compiler

Модуль компилятора генерирует и компилирует код в парадигме расширенных иерархических машин состояний на основе схем формата [`Cyberiada-GraphML`](https://docs.google.com/document/d/1UvEqwYkvqQq6VUECvo26BIcT7lWrcHJiOLKnwCazzPw/edit?tab=t.0#heading=h.52nvgrbeb8n5), получаемых по веб-сокету.
## Внешние зависимости
Установите `Python 3.10`.
### Arduino
Установите [`arduino-cli`](https://arduino.github.io/arduino-cli/dev/installation/) и добавьте его PATH. 
Затем введите команду:

`arduino-cli core install arduino:avr`
### МС-ТЮК
#### Windows
Установите GNU Arm Embedded Toolchain, он же [`gcc-arm-none-eabi`](https://developer.arm.com/downloads/-/gnu-rm) *(может потребоваться VPN)*, и добавьте его в PATH.
#### Linux
`sudo apt install gcc-arm-none-eabi`
### ИК-Путеводная
Установите GNU Arm Embedded Toolchain (см. выше) и добавьте его в PATH.

Также для сборки необходимы UNIX-утилиты (sh.exe, rm.exe, mkdir.exe, echo.exe, cp.exe) и GNU Make:
- Linux:
    - UNIX-утилиты доступны по умолчанию, GNU Make устанавливается из штатного пакетного менеджера
- Windows:
    - UNIX-утилиты доступны, например, в [`busybox`](https://frippery.org/busybox/) или [`msys2`](https://www.msys2.org)
    - GNU Make (тестировалось на 4.4.1) можно скачать [`здесь`](https://github.com/mbuilov/gnumake-windows) (скачанный `make` надо переименовать в `make.exe`), либо установить с помощью `msys2`
    - Обязательно добавьте всё вышеперечисленное в PATH!
  
## Установка и запуск
### Windows
- `py -m pip install -e .` или `poetry install` *(если установлен менеджер зависимостей poetry)*.
- `py -m compiler [флаги]`
### Linux
- `python3 -m pip install -e .` или `poetry install` *(если установлен менеджер зависимостей poetry)*.
- `python3 -m compiler [флаги]`
## Конфигурация
Конфигурация модуля может происходить через:
1. флаги, указываемые при запуске сервиса;
2. переменные окружения;
3. переменные в файле `compiler/config.py`.

Способы конфигурации указаны в порядке их приоритета.
> Внимание! Конфигурация по умолчанию выставляется для ОС Linux, поэтому пользователям Windows нужно поменять директорию создания проекта!

| Название                         | Описание                                                                                                                                                             | Флаг                   | Окружение                           | config.py             |
| -------------------------------- | ------------------------------------------------- | ----------------------------- | ----------------------------------- | --------------------- |
| **Порт хоста**                   | Порт, на котором будет доступен сервис                                                                                                                               | *--server-port*        | *LAPKI_COMPILER_SERVER_PORT*        | *_SERVER_PORT*        |
| **Адрес хоста**                  | Адрес, на котором будет доступен сервис                                                                                                                              | *--server-host*        | *LAPKI_COMPILER_SERVER_HOST*        | *_SERVER_HOST*        |
| **Директория создания проектов** | Директория, в которой будут создаваться проекты со сгенерированным кодом и используемыми библиотеками                                                                | *--build-directory*    | *LAPKI_COMPILER_BUILD_PATH*         | *_BUILD_DIRECTORY*    |
| Директория с библиотеками        | Директория, в которой находятся файлы реализации библиотек.                                                                | *--library-path*       | *LAPKI_COMPILER_LIBRARY_PATH*       | *_LIBRARY_PATH*       |
| Директория с платформами         | Директория, в которой находятся файлы-конфигурации платформ.| *--platform-directory* | *LAPKI_COMPILER_PLATFORM_DIRECTORY* | *_PLATFORM_DIRECTORY* |
| Лог-файл                         | Путь до лог-файла, если файл не существует, то он будет создан.                                                                                                      | *--log-path*           | *LAPKI_COMPILER_LOG_PATH*           | *_LOG_PATH*           |


## Запуск тестов
### Windows
- `py -m pip install -e .` или `poetry install` *(если установлен менеджер зависимостей poetry)*.
- `py -m pytest [флаги]`
### Linux
- `python3 -m pip install -e .` или `poetry install` *(если установлен менеджер зависимостей poetry)*.
- `python3 -m pytest [флаги]`

## Связанные проекты
- [Lapki IDE](https://github.com/kruzhok-team/lapki-client) - среда визуального программирования, в которую внедрен модуль компилятора.
- [cyberiadaml-py](https://github.com/kruzhok-team/cyberiadaml-py) - библиотека для парсинга Cyberiada-GraphML схем.
