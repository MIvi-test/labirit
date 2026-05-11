# Примеры использования (все платформы)

Здесь собраны примеры команд для разных операционных систем.

## Windows (MSYS2/PowerShell)

### Базовая сборка и запуск

```powershell
# Перейти в директорию проекта
cd C:\Users\YourName\labirit

# Собрать
mingw32-make compile

# Запустить
.\main.exe

# Быстрая проверка сборки
powershell -File .\build-test.ps1
```

### Полный цикл: очистка → сборка → тест → бенчмарк

```powershell
mingw32-make clean
mingw32-make compile
mingw32-make test
mingw32-make run-benchmarks
```

### Сборка с пользовательскими флагами

```powershell
# С оптимизацией
$env:CFLAGS = "-O2"
mingw32-make compile

# С максимальной оптимизацией (быстрее, но больше файл)
$env:CFLAGS = "-O3 -march=native"
mingw32-make compile

# Debug-версия с символами
$env:CFLAGS = "-g -O0"
mingw32-make compile
```

---

## Linux (Ubuntu/Debian/Fedora)

### Установка зависимостей

```bash
# Ubuntu/Debian
sudo apt-get install build-essential gcc make

# Fedora/RHEL
sudo dnf install gcc make
```

### Базовая сборка и запуск

```bash
# Перейти в директорию проекта
cd ~/Documents/labirit

# Проверить платформу
make show-platform

# Собрать
make compile

# Запустить
./main

# Запустить с выводом JSON
./main --dump

# Быстрая проверка сборки
bash ./build-test.sh
```

### Полный цикл

```bash
make clean
make compile
make test
make run-benchmarks
```

### Сборка с разными компиляторами

```bash
# С gcc (по умолчанию)
make compile

# С clang
CC=clang make compile

# С gcc-12 (если установлен)
CC=gcc-12 make compile
```

### Сборка с пользовательскими флагами

```bash
# С оптимизацией
CFLAGS="-O2" make compile

# С максимальной оптимизацией
CFLAGS="-O3 -march=native" make compile

# Debug версия
CFLAGS="-g -O0 -fsanitize=address" make compile
```

---

## macOS

### Установка зависимостей

```bash
# Если ещё не установлены Xcode Command Line Tools
xcode-select --install

# Или через Homebrew (если установлена)
brew install gcc make
```

### Базовая сборка и запуск

```bash
# Перейти в директорию проекта
cd ~/labirit

# Проверить платформу
make show-platform

# Собрать
make compile

# Запустить
./main

# Быстрая проверка сборки
bash ./build-test.sh
```

### Полный цикл

```bash
make clean
make compile
make test
make run-benchmarks
```

### Сборка с полной оптимизацией

```bash
# Для процессоров Apple Silicon (M1/M2/M3)
CFLAGS="-O3 -march=native -mtune=native" make compile

# Для Intel процессоров
CFLAGS="-O3 -march=native" make compile
```

### Использование GCC вместо Clang

```bash
# Установить gcc через Homebrew
brew install gcc

# Найти путь к gcc
which gcc-14  # или другая версия

# Собрать с gcc
CC=/usr/local/bin/gcc-14 make compile
```


## Отладка и диагностика

### Проверить текущую платформу

```bash
# Все платформы
make show-platform
mingw32-make show-platform  # Windows
```

### Просмотреть команды компиляции

```bash
# Выполняет компиляцию и показывает команды
mingw32-make compile VERBOSE=1  # Windows
make compile VERBOSE=1          # Linux/macOS
```

### Проверить установленные компиляторы

```bash
# Linux/macOS
which gcc
which clang
gcc --version
clang --version

# Windows
where gcc
gcc --version
```

### Очистить всё и пересобрать

```bash
mingw32-make clean    # Windows
make clean            # Linux/macOS

# А затем собрать
mingw32-make compile  # Windows
make compile          # Linux/macOS
```

---

## Типичные проблемы

### Windows: "mingw32-make: command not found"

**Решение:**

```powershell
# Добавить MSYS2 в PATH, либо использовать полный путь
C:\msys64\usr\bin\mingw32-make.exe compile

# Или отредактировать переменную окружения PATH
```

### Linux: "make: command not found"

**Решение:**

```bash
sudo apt-get install make
# или
sudo dnf install make
```

### macOS: "command not found: make"

**Решение:**

```bash
brew install make
# Или установить Xcode Command Line Tools
xcode-select --install
```

### Ошибка линковки на Windows: undefined reference

**Решение:**

Убедитесь, что mingw32-make добавляет флаги `-lgdi32 -luser32`:

```bash
mingw32-make show-platform
```

Если нет, проверьте переменную `PLATFORM` в Makefile.

---

## Дополнительные ссылки

- [BUILDING.md](BUILDING.md) — подробное руководство по сборке
- [README.md](README.md) — главное описание проекта
- [build-test.sh](build-test.sh) — скрипт для быстрого тестирования (Linux/macOS)
- [build-test.ps1](build-test.ps1) — скрипт для быстрого тестирования (Windows)
