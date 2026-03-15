# NoC-sim

Симулятор интерконнектов с различными топологиями, маршрутизациями, с возможностью симулировать различные сценарии трафика.

Само приложение реализовано в виде интерфейса, написанного на Java, работающего с dll на C++, где имплементирована симуляция.

## TODO:

Interface:
- парсинг строки параметров и их проверка на стороне C++
- обработка ошибок в канале interface <-> app
- single source of truth описания возможных конфигураций интерконнекта.

Interconnect:
  - виртуальные методы router: route_pkt
  - виртуальный метод итератора
  - виртуальные методы в Interconnect: build_topology, run_simulation, etc.


## Литература:

1. **Natalie Enright Jerger**, *On-Chip Networks*. Morgan & Claypool Publishers, 2017.

2. **William J. Dally** and **Brian Towles**, *Principles and Practices of Interconnection Networks*. Morgan Kaufmann, 2004.

3. **Grigoriy Rechistov**, *Программное моделирование вычислительных систем*. https://github.com/grigory-rechistov/simbook/raw/master/metoda/main-web.pdf
