# NoC Simulator JNI

Network-on-Chip симулятор с графическим интерфейсом на Java и вычислительным ядром на C++. Проект использует JNI (Java Native Interface) для связи между GUI и симуляционным движком.

## Возможности

- Симуляция Mesh топологии сети на кристалле
- Поддержка X-Y (STATIC) маршрутизации
- Равномерный (UNIFORM) и hotspot (HOTSPOT) паттерны трафика
- Графический интерфейс для настройки и запуска симуляций
- Визуализация результатов в реальном времени
- Логирование процесса симуляции

## Требования

### Для сборки:
- **C++ компилятор** с поддержкой C++17 (GCC 7+, Clang 5+)
- **CMake** 3.10 или выше
- **JDK** 8 или выше (для JNI заголовков и компиляции Java)
- **nlohmann/json** библиотека (устанавливается через пакетный менеджер)
- **Google Test** (опционально, для тестов)

### Для запуска:
- **JRE** 8 или выше
- **Linux** (основная платформа разработки) или **WSL** на Windows

### Установка зависимостей

#### Ubuntu/Debian:
```bash
sudo apt update
sudo apt install -y build-essential cmake default-jdk nlohmann-json3-dev libgtest-dev
```

## Сборка проекта

### Конфигурация CMake

Проект использует CMake для сборки. Доступны следующие опции:

| Опция | Описание | По умолчанию |
|-------|----------|--------------|
| `BUILD_JNI` | Собрать JNI модуль для связи Java и C++ | ON |
| `BUILD_TESTS` | Собрать тесты (Google Test) | ON |

### Базовая сборка

```bash
# Клонирование репозитория
git clone <repository-url>
cd NoC-sim

# Создание директории сборки
mkdir build
cd build

# Конфигурация с настройками по умолчанию
cmake ..

# Конфигурация с отключением тестов
cmake .. -DBUILD_TESTS=OFF

# Конфигурация с отключением JNI (только C++ библиотеки)
cmake .. -DBUILD_JNI=OFF

# Сборка
make -j$(nproc)

# генерация документации
mkdir ../docs
make docs
```

## Структура проекта

NoC-sim/
├── src/
│   ├── interconnect/                 # C++ ядро симуляции
│   │   ├── core/                     # Базовые классы (Packet, Router, Interconnect)
│   │   ├── mesh/                     # Mesh топология и роутеры
│   │   └── network/                  # Сетевая симуляция
│   ├── Interface/                    # Java GUI
│   │   ├── InterconnectApp.java      # Точка входа
│   │   ├── gui/                      # Компоненты GUI
│   │   ├── model/                    # Модели данных
│   │   ├── service/                  # Бизнес-логика и JNI
│   │   └── exception/                # Исключения
│   ├── JNI/                          # JNI обёртка
├── tests/                            # Тесты (GTest)
├── lib/                              # JSON библиотека для Java

JNI: После компиляции Java исходников генерируем header с сигнатурами методов, которые нужно реализовать со стороны C++. После часть на C++ компилируется как динамическая библиотека (.so) и в runtime линкуется к приложению, написанному на Java. (см. [src/JNI/CMakeLists.txt](JNI_CMake))


## Литература:

1. **Natalie Enright Jerger**, *On-Chip Networks*. Morgan & Claypool Publishers, 2017.

2. **William J. Dally** and **Brian Towles**, *Principles and Practices of Interconnection Networks*. Morgan Kaufmann, 2004.

3. **Grigoriy Rechistov**, *Программное моделирование вычислительных систем*. https://github.com/grigory-rechistov/simbook/raw/master/metoda/main-web.pdf
