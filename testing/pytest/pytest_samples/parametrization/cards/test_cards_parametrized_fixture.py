import pytest
from cards import Card

@pytest.fixture(params=['done', 'in prog', 'todo'])
def start_state(request):
  return request.param

def test_finish(cards_db, start_state):
  card_id = cards_db.add_card(Card('card_name', state=start_state))
  cards_db.finish(card_id)
  assert cards_db.get_card(card_id).state == 'done'

