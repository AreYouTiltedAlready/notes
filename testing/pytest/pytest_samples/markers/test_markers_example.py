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
