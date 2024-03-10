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

def test_zero_stupid():
  with TemporaryDirectory() as db_dir:
    db_path = Path(db_dir)
    db = cards.CardsDB(db_path)

    count = db.count()
    db.close()

    assert count == 0

def test_addition(cards_db):
  cards_db.add_card(cards.Card('card 1'))
  cards_db.add_card(cards.Card('card 2)'))
  assert cards_db.count() == 2

def test_zero(cards_db):
  assert cards_db.count() == 0
