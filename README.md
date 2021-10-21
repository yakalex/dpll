# Алгоритм DPLL
## Сборка
`make dpll`
## Запуск
`./dpll path_to_file`
`path_to_file` - путь к файлу в формате [DIMACS](https://www.cs.ubc.ca/~hoos/SATLIB/benchm.html)
## Результаты
Тестировалось на формуле **hanoi4.cnf**:
* Время работы ~13c (один поток, intel core i5 8250U)
* Пиковое потребление памяти ~3МБ