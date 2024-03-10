import cards

def test_addition(cards_db):
  cards_db.add_card(cards.Card('card 1'))
  cards_db.add_card(cards.Card('card 2)'))
  assert cards_db.count() == 2

def test_zero(empty_cards_db):
  assert empty_cards_db.count() == 0
