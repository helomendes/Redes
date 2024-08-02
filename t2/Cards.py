import random

class Cards:
    def __init__(self):
        self.deck = []

    def set_deck(self):
        num = ['3', '2', 'A', 'K', 'J', 'Q', '7', '6', '5', '4', 'C']
        # QUESTION: só tem um coringa? Ou um de cada naipe?

        naipes = ['♠️', '♦️', '♥️', '♣️']
        self.deck = [f'{j}  {i}' for j in naipes for i in num]
        random.shuffle(self.deck)

    def choose_cards(self, ROUND):
        random.shuffle(self.deck)
        for player in range(4):

