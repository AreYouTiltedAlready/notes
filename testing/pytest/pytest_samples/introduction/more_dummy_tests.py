def test_less() -> None:
  a = 6
  b = 5
  assert a < b

def test_contains() -> None:
  assert 1 in [2, 3, 4, 5]

def test_contains_str() -> None:
  assert 'fizz' not in 'fizzbuzz'
