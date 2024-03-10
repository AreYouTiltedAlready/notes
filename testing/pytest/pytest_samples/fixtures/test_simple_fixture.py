import pytest

@pytest.fixture()
def some_data() -> int:
  return 42

def test_some_data(some_data) -> None:
  assert some_data == 42
