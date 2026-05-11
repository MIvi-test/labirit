# Инструкции по сборке проекта (кроссплатформенно)

Проект поддерживает сборку на **Windows**, **Linux** и **macOS** благодаря Makefile.

## Автоматическое определение платформы

Makefile автоматически определяет операционную систему и применяет необходимые флаги и библиотеки компилятора.

---

## Windows (MSYS2 / MinGW)

### Требования

1.  **MSYS2** установлен на систему
2.  **gcc** (уже идёт с MSYS2)
3.  **mingw32-make** (входит в MSYS2)

### Установка (если MSYS2 ещё не установлена)

```powershell
# Загрузить с https://www.msys2.org/# Или через Chocolatey:
choco install msys2
```

### Сборка

```bash
# Сборка основной программы
mingw32-make compile
mingw32-make run
# Запуск тестов
mingw32-make test
# Запуск бенчмарков
mingw32-make run-benchmarks
# Очистка
mingw32-make clean
```

### Особенности Windows сборки

-   Автоматически добавляются флаги: `-D_WIN32_WINNT=0x0601`
-   Линкуются библиотеки: `-lgdi32 -luser32` (для GUI)
-   Расширение исполняемого файла: `.exe`
-   Команда удаления: `del /Q`

---

## Linux (Ubuntu/Debian/Fedora)

### Требования

```bash
# Ubuntu/Debian
sudo apt-get install build-essential gcc make
# Fedora/RHEL
sudo dnf install gcc make
```

### Сборка

```bash
# Сборка основной программы
make compile
make run
# Запуск тестов
make test
# Запуск бенчмарков
make run-benchmarks
# Очистка
make clean
```

### Особенности Linux сборки

-   Автоматически добавляются флаги: `-D_GNU_SOURCE`
-   Дополнительная библиотека: `-ldl`
-   Без расширения для исполняемого файла
-   Команда удаления: `rm -f`

---

## macOS

### Требования

```bash
# Установить Xcode Command Line Tools
xcode-select --install
# Или установить Homebrew и необходимые пакеты
brew install gcc make
```

### Сборка

```bash
# Убедитесь, что используется GNU make (не BSD make)
which make
 # должен указывать на /usr/local/bin/make или из Homebrew
 # Сборка основной программы
 make compilemake run
 # Запуск тестов
 make test
 # Запуск бенчмарков
 make run-benchmarks
 # Очисткаmake clean
```

### Особенности macOS сборки

-   Автоматически добавляются флаги: `-D__APPLE__`
-   Без дополнительных библиотек (только `-lm`)
-   Без расширения для исполняемого файла
-   Команда удаления: `rm -f`

---

## Переменные окружения

Вы можете переопределить переменные компилятора:

```bash
# Использовать другой компилятор
CC=clang make compile
# Добавить дополнительные флаги
CFLAGS="-O3 -march=native" make compile
# Всё вместе
CC=gcc-12 CFLAGS="-O2" make compile
```

---

## Решение проблем

### Ошибка: “mingw32-make: command not found” (Windows)

```bash
# Убедитесь, что MSYS2 в PATH
# Или используйте полный путь:
C:\msys64\mingw64\bin\mingw32-make.exe compile
```

### Ошибка: “make: command not found” (Linux/macOS)

```bash
# Ubuntu/Debian
sudo apt-get install make
# Fedora
sudo dnf install make
# macOS
brew install make
```

### Ошибка линковки на Windows

Убедитесь, что mingw32-make автоматически добавляет `-lgdi32 -luser32`. Если нет, проверьте переменную `PLATFORM` в Makefile.

### Различия между платформами при запуске

Программа автоматически адаптируется благодаря условной компиляции. Проверьте флаги в исходном коде:

```c
#ifdef _WIN32      
 // Windows
 #ifdef _GNU_SOURCE  
 // Linux#
 ifdef __APPLE__    
 // macOS
```

---

## Дополнительные команды


```bash
# Показать текущую платформу и настройки
make show-platform
# Полная переборка
make cleanmake compile
# Запуск с дополнительным выводом (debug)
CFLAGS="-Wall -Wextra -Werror -std=c23 -g -O0"
make compile
```