## Более кратко про стек:
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