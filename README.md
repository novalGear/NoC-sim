# NoC-sim

Симулятор интерконнектов с различными топологиями, маршрутизациями, с возможностью симулировать различные сценарии трафика.

Само приложение реализовано в виде интерфейса, написанного на Java, работающего с dll на C++, где имплементирована симуляция.

## TODO:

- парсинг строки параметров и их проверка на стороне C++
- обработка ошибок в канале interface <-> app
- single source of truth описания возможных конфигураций интерконнекта.

## Литература:

1. **Natalie Enright Jerger**, *On-Chip Networks*. Morgan & Claypool Publishers, 2017.

2. **William J. Dally** and **Brian Towles**, *Principles and Practices of Interconnection Networks*. Morgan Kaufmann, 2004.

