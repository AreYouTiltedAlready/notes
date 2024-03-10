import pytest
import cards

from pathlib import Path
from tempfile import TemporaryDirectory

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
