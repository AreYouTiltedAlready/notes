import pytest
from cards import Card

@pytest.mark.parametrize('card_name, start_state', [('todo_card', 'todo'), ('in_prog_card', 'in prog'), ('done_card', 'done')])
def test_finish(cards_db, card_name, start_state):
  card_id = cards_db.add_card(Card(card_name, state=start_state))
  cards_db.finish(card_id)
  assert cards_db.get_card(card_id).state == 'done'

