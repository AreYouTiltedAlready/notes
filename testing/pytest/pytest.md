# Pytest

## Введение

### Quick start
Удобнее всего работать в отдельной виртуальной среде. Создаётся она так:

```bash
python3 -m venv venv
source venv/bin/activate
```

Установим сам `pytest`:
```bash
pip install pytest
```

Готово!

### Пример: простой тест
```bash
mkdir dummy_test && cd dummy_test
touch dummy_test.py
```

В `dummy_test.py` напишем такой тест:
```python
def test_dummy() -> None:
  assert (1, 2, 3) == (3, 2, 1)
```

И запустим `pytest`:
```bash
pytest -v dummy_test.py # --verbose
```

Увидим падающий тест и довольно подробное описание того, что не так.

### О том, как pytest ищет, что выполнять
* Файлы с тестами должны называться `<something>_test.py` или `test_<something>.py`
* Тестирующие функции должны называться `test_<something>`
* Тестирующие классы должны называться `Test<Something>`

### Ещё немножко примеров
Можно посмотреть в `introduction/more_dummy_tests.py`

## Структура тестов
Есть мнение, что `assert`ы лучше использовать в конце теста (и только там). Это очень распространённая практика, причём настолько, что у неё есть два названия: *Arrange-Act-Assert* и *Given-When-Then* (всё довольно мнемонично).

Понятно, что не столь важны названия, идея в том, чтобы разбить тест на некоторые этапы:

1. Подготовка данных для теста
2. Совершение с этими данными некоторых действий
3. Проверка того, что результат совпадает с ожидаемым

Небольшой пример:
```python
# Возвращаем для строки суффиксный массив
def sa_naive(s: str) -> list[int]:
  n = len(s)
  order = [_ for _ in range(n)]
  order.sort(key=lambda i: s[i:])
  return order

def test_sa_naive() -> None:
  # Arrange
  my_str = 'aaaaa'
  # Act
  sa_result = sa_naive(my_str)
  # Assert
  assert sa_result == [4, 3, 2, 1, 0]
```

Из идейного - можно группировать тесты по классам:
```python
class TestSuffixArray:
  def test_sa_naive(self):
    ...
  
  def test_sa_doubling(self):
    ...

  def test_sa_is(self):
    ...
```

### Как запускать некоторое подмножество тестов
Допустим, у нас есть `test_source_dir`, в которой лежат какие-то файлы с тестами:
```bash
# запуск всех тестов в директории
pytest test_source_dir

# запуск всех тестов в конкретном файле
pytest test_source_dir/test_sa.py

# запуск всех тестов в конкретном классе
pytest test_source_dir/test_sa.py::TestSuffixArray

# запуск конкретного теста в конкретном классе
pytest test_source_dir/test_sa.py::TestSuffixArray::test_sa_naive

# запуск конкретного теста (если это отдельная функция, а не метод класса)
pytest test_source_dir/test_sa.py::test_sa_naive

# запуск тестов в test_source_dir, чьи имена матчатся по regex
pytest test_source_dir -k *regex*
```

## Фикстуры
В чем идея?  
Ну, чаще всего мы тестим что-то довольно нетривиальное, и нам нужно подготовить какие-то данные (которые могут быть общими для некоторых тестов), или привести систему в то состояние, в котором мы хотим её тестировать, ...  

### Простой пример

Всё это есть в файле `fixtures/test_simple_fixture.py`
```python
import pytest

@pytest.fixture()
def some_data() -> int:
  return 42

def test_some_data(some_data) -> None:
  assert some_data == 42

```
Важно для диагностирования ошибок: если тест завершается с вердиктом `FAIL` - ошибка в самом тесте, а если с вердиктом `ERROR` - где-то в фикстуре.  

### SetUp - TearDown фикстуры
Да, концепция довольно общая (используется и в `GTest`). Для примера поставим себе простое приложение и поиграемся с ним:
```bash
pip install cards
```
Есть какой-то CLI:
```bash
cards add A
cards add B
cards count
# Output: 2

cards list
# Выведет все добавленные карты
```
Напишем тест, который будет проверять, что изначально `count` возвращает 0
```python
# fixtures/test_cards.py

import cards
from pathlib import Path
from tempfile import TemporaryDirectory

def test_zero_stupid():
  with TemporaryDirectory() as db_dir:
    db_path = Path(db_dir)
    db = cards.CardsDB(db_path)

    count = db.count()
    db.close()

    assert count == 0
```
Сам тест написать не очень больно - он занимает буквально несколько строк. И всё же... Какие проблемы есть при использовании такого подхода?  
* Мы инициализируем внутреннюю бд в модуле `Cards`, потом берём оттуда какую-то информацию, потом закрываем её - действий как-то много, учитывая то, что мы не тестим не  открытие/закрытие, а начальное состояние системы.
* Нам нужно сохранять результат вызова `count`, закрывать бд и потом уже `assert`ить что-то. Хочется наоборот, но не получается, потому что при срабатывании `assert`а бд не закроется

Решается это таким элегантным способом:
```python
# fixtures/test_cards.py

import pytest
import cards

from pathlib import Path
from tempfile import TemporaryDirectory

@pytest.fixture()
def cards_db():
  with TemporaryDirectory() as db_dir:
    db_path = Path(db_dir)
    db = cards.CardsDB(db_path)
    yield db
    db.close()

def test_zero(cards_db):
  assert cards_db.count() == 0
```

Да, генераторы - мощная штука.
Код в функциях, задекорированных с помощью `@pytest.fixture()`, исполняется так:  
* Интерпретатор начинает выполнение функции **до** начала выполнения тестов
* Как только он доходит до `yield`, состояние функции сохраняется (`yield`ится какой-то объект в каком-то состоянии), управление передаётся тестам
* Часть после `yield` выполняется после того, как закончится скоуп фикстуры (об этом ниже)

То есть первый пункт - *SetUp*, третий - *TearDown*.  
Когда мы передаём в тест аргумент с названием, например, `arg_name`, `pytest` ищет фикстуру с таким именем и прокидывает её в наш тест - всё очень просто.  

**Важно!** Фикстуры возвращаются в своё изначальное состояние в начале запуска каждого отдельного теста, то есть модификации фикстуры видны в рамках одного теста, но не видны за ним.

Пример:

```python
# fixtures/test_cards.py

# То, что мы добавляем в первом тесте, не видно во втором
def test_addition(cards_db):
  cards_db.add_card(cards.Card('card 1'))
  cards_db.add_card(cards.Card('card 2)'))
  assert cards_db.count() == 2

def test_zero(cards_db):
  assert cards_db.count() == 0
```

Полезная штука для просмотра конфигураций `SetUp/TearDown`: `pytest --setup-show test_cards.py`. Вывод в нашем примере будет такой:

```
test_cards.py 
        test_cards.py::test_zero_stupid.
        SETUP    F cards_db # Сетап фикстуры под именем cards_db
        test_cards.py::test_addition (fixtures used: cards_db).
        TEARDOWN F cards_db # TearDown той же структуры, дальше всё аналогично
        SETUP    F cards_db
        test_cards.py::test_zero (fixtures used: cards_db).
        TEARDOWN F cards_db
```
### Область видимости фикстуры
В примере выше она указывается сразу после слов `SetUp` и `TearDown`: *F* в этом примере означает *Function*, то есть дефолтный скоуп фикстуры - это одна функция.  
Все возможные скоупы:  
* **F** - *function* (default)
* **C** - *class*
* **M** - *module*
* **P** - *package*
* **S** - *session*  

#### Зависимость фикстур
Очень легко реализовать, просто в аргументах зависимой фикстуры прописать зависимость. Единственное, что нужно помнить - зависимая фикстура не может иметь более широкий скоуп, нежели фикстура-зависимость. Например, такой код работать не будет:
```python
@pytest.fixture(scope='function')
def A():
  ...

@pytest.fixture(scope='session')
def B(A):
  ...
```

### Вынесение фикстур в `conftest.py`
Это специальный файл, который автоматически читается `pytest`ом, т.е. нам ничего не нужно дополнительно импортировать. Туда можно (да и нужно, чтобы не засорять тестовые файлы) выносить общие фикстуры для каких-то тестов.  
Сейчас мы именно это и сделаем: создадим директорию `fixtures/configured/cards/`, скопируем туда всё и слепим `conftest.py`:

```bash
mkdir configured && cd configured
mkdir cards && cd cards
touch conftest.py test_cards.py
```

Например, мы хотим использовать один экземпляр `CardsDB` для всех тестов, но где-то хотим тестить пустую `CardsDB`. Тогда мы можем сделать следующее:

```python
# fixtures/configured/cards/
@pytest.fixture(scope='session')
def cards_db():
  with TemporaryDirectory() as db_dir:
    db_path = Path(db_dir)
    db = cards.CardsDB(db_path)
    yield db
    db.close()

@pytest.fixture(scope='function')
def empty_cards_db(cards_db):
  cards_db.delete_all()
  return cards_db
```
И использовать `empty_cards_db`, когда нам нужна пустая база данных:

```python
# fixtures/configured/cards/test_cards.py
import cards

def test_addition(cards_db):
  cards_db.add_card(cards.Card('card 1'))
  cards_db.add_card(cards.Card('card 2)'))
  assert cards_db.count() == 2

def test_zero(empty_cards_db):
  assert empty_cards_db.count() == 0
```

Чтобы посмотреть список всех фикстур, можно в рабочей директории вызвать `pytest --fixtures`. Правда, для наших фикстур мы увидим только названия и скоуп - описание нужно добавлять самим в функции, например:

```python

@pytest.fixture(scope='session')
def cards_db():
  """ Creates object connected to a temporary database"""
  with TemporaryDirectory() as db_dir:
    db_path = Path(db_dir)
    db = cards.CardsDB(db_path)
    yield db
    db.close()

@pytest.fixture(scope='function')
def empty_cards_db(cards_db):
  """CardsDB object that's empty"""
  cards_db.delete_all()
  return cards_db
```

Теперь при вызове `pytest --fixtures` увидим:

```
------------------------------------------------------------------------------ fixtures defined from conftest ------------------------------------------------------------------------------
empty_cards_db -- conftest.py:17
    CardsDB object that's empty

cards_db [session scope] -- conftest.py:8
    Creates object connected to a temporary database
```

Ну и взглянем на конфигурацию `SetUp/TearDown`:
```bash
pytest --setup-show
```
Вывод:
```
 test_cards.py 
SETUP    S cards_db
        test_cards.py::test_addition (fixtures used: cards_db).
        SETUP    F empty_cards_db (fixtures used: cards_db)
        test_cards.py::test_zero (fixtures used: cards_db, empty_cards_db).
        TEARDOWN F empty_cards_db
TEARDOWN S cards_db 
```

### Прочие трюки с фикстурами
Есть еще некоторые опции, которые могут оказаться полезными.

#### autouse
```py
import time

@pytest.fixture(autouse=True)
def time_measure_after_each_test():
  start_time = time.time()
  yield
  finish_time = time.time()
  print("\ntest duration : {:0.3} seconds".format(delta))

def test_foo():
  ...

def test_bar():
  ...
```

Фикстура `time_measure_after_each_test` будет выводить время, которое занял каждый тест.
**Важно!** Чтобы `pytest` выводил сообщения фикстур, нужно запускать с флагом `-s` (краткая версия `--capture=no`).

#### Переименовывание фикстур

```py
from bar_module import bar

@pytest.fixture(name='bar')
def bar_fixture():
  ...

# Use `bar_fixture` as `bar`
def test_bar(bar):
  ...
```

#### Нативные фикстуры `pytest`а
Рассмотрим пару `tmp_path`/`tmp_path_factory`:
#### tmp_path
* возвращает инстанс `pathlib.Path` (а в случае фикстур, на самом деле, служит алиасом) для некоторого пути в файловой системе, который поддерживается pytest'ом и живет в течение тестсьюта.
* `scope`: `function`

#### tmp_path_factory
* возвращает, как нетрудно догадаться, фабрику для создания путей.
* единственный use case:
```py
def test_something(tmp_path_factory):
  path = tmp_path_factory.mksub('sub')
  my_file = path / 'my_file.txt'
  # do something with file
```
* `scope` = `session`

#### capsys
Это уже более идейная штука. Часто бывает так, что мы хотим ловить `stdout`/`stderr` приложения, и по умолчанию `pytest` ловит этот вывод.  
Используется так:  
1. Пусть мы хотим поймать вывод приложения.
Например, те самые `cards` умеют сообщать свою версию:
```bash
cards version
# Output: v1.0.0
```
Можем протестить это двумя способами:
```py
import cards
import subprocess

def test_version_v1(capsys):
  process = subprocess.run(['cards', 'version'], capture_output=True, text=True)
  output = process.stdout.rstrip() # rstrip нужен, чтобы убрать '\n'
  assert output == cards.__version__
```
Или
```py
import cards
def test_version_v2(capsys):
  cards.cli.version()
  output = capsys.readouterr().out.rstrip()
  assert output == cards.__version__
```
2. Нам нужно поймать вывод какого-то теста. Тогда можно использовать такой трюк:

```py
def test_disabled(capsys):
  with capsys.disabled():
    print("\ncapsys disabled print") # это будет видно в выводе pytestа
```

#### monkeypatch
Это уже какая-то попытка в mocking - фикстура предоставляет такие методы:
* `setattr(target, name, value, raising=True)`: устанавливает какой-то атрибут в какое-то значение
* `delattr(target, name, raising=True)`: удаляет атрибут
* `setitem(dic, name, value)`: устанавливает значение в `dict`
* `delitem(dic, name, raising=True)`: удаляет значение в `dict`
* `setenv(name, value, prepend=None)`: устанавливает значение какой-то переменной окружения
* `delenv(name, raising=True)`: удаляет переменную окружения
* `syspath_prepend(path)`: добавляет путь в список include-путей интерпретатора
* `chdir(path)`: меняет рабочую директорию на указанный путь

## Параметризация
Мотивация: больше тестов богу тестов (а писать большое количество тестов руками как-то не хочется). В `pytest` есть 3 способа параметризации:  
* Параметризация тесткейсов
* Параметризация фикстур
* Волшебный хук `pytest_generate_tests`  

Здесь будут рассмотрены все три варианта, причём для каждого из них существуют случаи, когда он удобнее остальных.

В качестве примера у нас будут всё те же `Cards`.  
Допустим, мы хотим проверить метод `db.finish(index)`, который должен устанавливать значение карты с переданным индексом в *done*. Карта изначально может быть в одном из трёх состояний: *todo*, *in prog* или *done*.

Первое, что приходит в голову:
```py
from cards import Card

def test_finish(cards_db):
  for card in [Card('todo_task', state='todo'), Card('in_progress_task', state='in prog'), Card('finished_task', state='done')]:
    card_id = cards_db.add_card(card)
    cards_db.finish(card_id)
    assert cards_db.get_card(card_id).state == 'done'
```

Вариант рабочий, но не лишённый проблем:  
* Отчёт мы видим по одному тесткейсу (хотя вообще их как бы.. три)
* Если на какой-то итерации цикла тест упал, мы не знаем, на какой именно, не посмотрев логи
* После упавшей итерации тест завершается, что грустно, мы же хотим прогнать всё

На самом деле вариант выше уже достаточно хорош, проблемы минорны, именно так очень часто и пишут в реальном тестовом коде. Но можно лучше - давайте разбираться, как.  

### Параметризация тесткейса
Делается это вот так:
```py
import pytest
from cards import Card

@pytest.mark.parametrize('start_state', ['todo', 'in prog', 'done'])
def test_finish(cards_db, start_state):
  card_id = cards_db.add_card(Card('card_name', state=start_state))
  cards_db.finish(card_id)
  assert cards_db.get_card(card_id).state == 'done'
```
А можно добавить и какой-то аргумент для имени, тогда `(arg_1, arg_2)` нужно будет обернуть в `tuple`:
```py
import pytest
from cards import Card

@pytest.mark.parametrize('card_name, start_state', [('todo_card', 'todo'), ('in_prog_card', 'in prog'), ('done_card', 'done')])
def test_finish(cards_db, card_name, start_state):
  card_id = cards_db.add_card(Card(card_name, state=start_state))
  cards_db.finish(card_id)
  assert cards_db.get_card(card_id).state == 'done'
```

**Важно!** Аргументы здесь указываются одной строкой.

### Параметризация фикстур
Можно сделать фикстуру, для которой сгенерится несколько разных значений. Тогда все тесты, использующие эту фикстуру, тоже будут исполнены несколько раз:
```py
import pytest
from cards import Card

@pytest.fixture(params=['done', 'in prog', 'todo'])
def start_state(request):
  return request.param

def test_finish(cards_db, start_state):
  card_id = cards_db.add_card(Card('card_name', state=start_state))
  cards_db.finish(card_id)
  assert cards_db.get_card(card_id).state == 'done'
```
Когда параметризовывать тесткейсы, а когда - фикстуры? Есть эмпирическое правило:  
* *один тест - разные данные* - параметризация тесткейсов
* *один тест - разное стартовое состояние* - параметризация фикстур

### Параметризация с помощью `pytest_generate_tests`
На самом деле, это читерский способ. Реализовывается так: в `conftest.py` добавляем:
```py
def pytest_generate_tests(metafunc):
  if 'start_state' in metafunc.fixturenames:
    metafunc.parametrize('start_state', ['todo', 'in prog', 'done'])
```
Реализация `test_finish` не меняется:
```py
def test_finish(cards_db, start_state):
  card_id = cards_db.add_card(Card('card_name', state=start_state))
  cards_db.finish(card_id)
  assert cards_db.get_card(card_id).state == 'done'
```

## Маркеры
Маркеры - способ подсказать фреймворку что-то конкретное про отдельно взятый тест. Хорошим синонимом в этом контексте будут "тэг" или "метка".  
Примеры:
* Если какие-то из тестов уж очень медленные, можно повесить на них `@pytest.mark.slow` и заставить `pytest` их скипнуть
* *Smoke-тесты* можно пометить как `@pytest.mark.smoke` и сделать их запуск первым стейджем в CI, например (smoke-тесты - это необходимый минимум для проверки работоспособности системы)

Маркеры можно разделить на две категории: нативные маркеры, как-то меняющие поведение фреймворка на помеченном ими тесте, и кастомные, которые мы (зачем-то) создаём сами. Рассмотрим оба типа.

### Нативные маркеры
* `@pytest.mark.filterwarnings(warning)` - добавляет фильтр предупреждений к текущему тесту
* `@pytest.mark.skip(reason=None)` - Пропускаем тест, (опционально) указывается причина
* `@pytest.mark.skipif(condition, ..., *, reason)` - Пропускаем тест, если выполнено какое-то из условий
* `@pytest.mark.xfail(condition, ..., *, reason, run=True, raises=None, strict=xfail_strict)` - Сообщаем фреймворку, что ожидаем, что этот тест упадёт
* `@pytest.mark.parametrize(argnames, argvalues, indirect, ids, scope)` - мы уже в курсе - параметризация
* `@pytest.mark.usefixtures(fixturename1, fixturename2, ...)` - означает, что тест использует все перечисленные фикстуры

Наиболее часто использующиеся:
* `@pytest.mark.parametrize()`
* `@pytest.mark.skip()`
* `@pytest.mark.skipif()`
* `@pytest.mark.xfail()`

С `@pytest.mark.parametrize()` уже разобрались, посмотрим на остальные три:

#### @pytest.mark.skip(if)?
```py
import pytest

@pytest.mark.skip(reason='Why not?')
def test_addition():
  a = 5
  b = 7
  assert a + b == 12

@pytest.mark.skipif(5 > 7, reason='Why not x2?')
def test_subtraction():
  a = 5
  b = 7
  assert a - b == -2
```

Запустив `pytest`, увидим:
```
collected 2 items                                                                                                                                                                          

test_markers_example.py::test_addition SKIPPED (Why not?)                                                                                                                            [ 50%]
test_markers_example.py::test_subtraction PASSED                                                                                                                                     [100%]

=============================================================================== 1 passed, 1 skipped in 0.00s ===============================================================================    
```

#### @pytest.mark.xfail()
Полная сигнатура выглядит так: `@pytest.mark.xfail(condition, ..., *, reason, run=True,
raises=None, strict=xfail_strict)`  

С помощью параметра `run` можно в целом выключить тест, а вот последний аргумент - `strict` - уже интереснее. Если тест падает, всё вроде бы ок - мы этого и ожидали. Если же он проходит, то не очень понятно, насколько это нормальное поведение и что `pytest` должен вывести. Для этого и есть параметр `strict` - если в вышеописанной ситуации он равен `True`, то тест завершится с вердиктом `FAIL`, если `False` - то с вердиктом `XPASS`

Вообще прагматичный подход - *всегда* выставлять `strict=True` довольно логичен, потому что в этом случае тест завершится с `FAIL`, если поведение программы не совпадает с нашими ожиданиями.

### Кастомные маркеры и прогонка подмножества тестов
Первое, что нужно отметить - все кастомные теги нужно сохранять в файл `pytest.ini` в таком формате:
```
[pytest]
markers:
    smoke: subset of tests
    exception: check for expected exceptions
```
И все тесты, помеченные тегом `smoke`, например, мы можем запустить так: `pytest -v -m smoke`

Если при запуске `pytest` не находит в `pytest.ini` переданного тега, выскочит предупреждение типа
```
PytestUnknownMarkWarning: Unknown pytest.mark.smoke - is this a typo?
```
Это спасает нас от опечаток в названиях тегов (например, `pytest -v -m smok`)  
Чтобы сделать такой ворнинг ошибкой, нужно запускать `pytest` с флагом `--strict-markers`.
Тегом можно помечать не только функции, но и классы, и целые файлы:

```py
@pytest.mark.somemark
class TestSomething:
  ...
```
```py
...
pytestmark = pytest.mark.somemark
...
```
Последний пример тегает весь файл.  
В параметризации можно тегать отдельные параметры, например:
```py
@pytest.mark.parametrize(
"start_state", ["todo", pytest.param("in prog", marks=pytest.mark.smoke), "done"]
)
def test_something(..., start_state):
  ...
```
Аналогичным образом тегаются параметры фикстур.

### Ещё про `pytest.ini`
Примерная конфигурация может выглядеть так
```
[pytest]
markers =
    <marker_name>: <marker_description>
    <marker_name>: <marker_description>

addopts =
    --strict-markers
    -ra

xfail_strict = true
```
`addopts` - удобная штука (туда можно передавать и `-s`, например)

