# labirit

Небольшой исследовательский проект по генерации и анализу лабиринтов.

## Что делает программа

- генерирует лабиринты алгоритмами `prim`, `growing_tree`, `watson`;
- запускает серию тестов по времени и топологии;
- сохраняет результаты в JSON;
- по флагу `--dump` выводит собранные JSON в консоль.

## Структура

- `backend/` — алгоритмы и базовые структуры;
- `analisys/` — код анализа и каталог результатов `json_data`;
- `json_parser.c` и `json_parser.h` — вывод JSON-результатов;
- `main.c` — сценарий запуска эксперимента.

## Использование

```bash
make compile
./main
./main --dump
```

Результаты сохраняются в `analisys/json_data`.

## Тестирование (Windows + Linux)

Тесты прогоняют **все** алгоритмы генерации и проверяют:
- симметрию стен между соседними клетками;
- закрытые внешние границы;
- связность лабиринта (из (0,0) достижимы все клетки).

Сборка и запуск:

### Linux/macOS (bash)

```bash
gcc -std=c11 -O2 -Wall -Wextra -Wpedantic -o tests_run tests/test_algorithms.c
./tests_run
```

### Windows (PowerShell)

```powershell
gcc -std=c11 -O2 -Wall -Wextra -Wpedantic -o tests_run.exe tests/test_algorithms.c
.\tests_run.exe
```

Можно прогнать один алгоритм:

```bash
./tests_run --alg prim
```
