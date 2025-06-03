# [Для более короткого стека вот ссылка](#First) 

## Примерно намечу стек технологий которые будут нужны при работе

## Языки и компиляторы
* С++ от 17 стандарта и выше
* C++/CLI или C++/WinRT (нужно разобраться)

## Системы сборки и управления зависимостями
* CMake (По базе)
* CPack (часть CMake) - нужно более детально разобраться
* Git (По базе для управления)

## Сетевое взаимодействие
* Boost.Asio (Boost 1.66 и выше) - для работы с сокетами(опять же нужно крепить доки ко всему для себя)
* Winsock (Winsock2.h) и POSIX - сокеты

## Сериалиазиция и протокол обмена
* nlohmann/json - (снова же разбираться)
* JSON-RPC-протокол - как я понимаю унифицированный формат запросов и ответов

## Работа с пользователями и группами
* Linux (glibc)
* Windows(Win32 API/ NetUser API/ WMI)

## Работа с файловой системой и ACL - (Полностью нужно будет чекать блок)
* C++17 std::filesystem
* Linux (libacl)
* Windows (WinAPI-функции для безопасности)

## Управление демонами и службами
* Linux (systemd / SysV init)
* Windows (Service Control Manager)

## Сбор информации о «железе» и ресурсах
* Linux (procfs, sysfs, lm-sensors, smartmontools)
* Windows (WMI / PDH / Performance Counters)

## Графический интерфейс (GUI)
* Qt5 - (По базе)

## Логирование
* spdlog - как в ентернете написана более легкая и быстрая
* Boost.Log - опять же в ентернете говорят "родная", но тяжолая

## Тестирование
* Google Test (gtest) / Catch2
* Boost.Test

## Упаковка и развёртывание
* Linux-паки (.deb) CPack с генератором DEB: настройка CPACK_DEBIAN_PACKAGE_DEPENDS, скрипты preinst, postinst, unit-файл systemd.
* Windows-инсталляторы (NSIS или WiX) Сценарии для установки/удаления: копирование исполняемых файлов, необходимых DLL (Boost, Qt), PowerShell-скрипты установки службы (InstallService.ps1) и ярлыки.

## Дополнительные утилиты и инструменты
* lm-sensors, smartmontools (Linux)
* WiX Toolset (Windows-MSI) или NSIS
* Make / NMake / Ninja


<span id="First"></span>
## **Более кратко про стек:**
* Язык: C++17 (gcc/Clang для Linux, MSVC/MinGW для Windows).

* Сборка: CMake + CPack (DEB, NSIS/WiX).

* Сетевое взаимодействие: Boost.Asio (или POSIX/WinSock).

* JSON: nlohmann/json.

* ACL-функции: libacl (Linux), WinAPI (Windows).

* Управление пользователями/группами: glibc (getpwent, getgrent) + утилиты (useradd, groupadd) на Linux; NetUser API / WMI на Windows.

* Мониторинг ресурсов: procfs/sysfs + lm-sensors + smartmontools на Linux; WMI / PDH / Performance Counters на Windows.

* Управление демонами/службами: systemd (Linux), SCM (WinAPI) + PowerShell (Windows).

* GUI: Qt5/Qt6.

* Логирование: spdlog (или Boost.Log).

* Тестирование: Google Test / Catch2 (модульные тесты).

* Упаковка: DEB (Linux), NSIS/WiX (Windows).
