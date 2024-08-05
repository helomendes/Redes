import random

class Cards:
    def __init__(self):
        self.deck = []
        self.set_deck()

    def set_deck(self):
        num = ['3', '2', 'A', 'K', 'J', 'Q', '7', '6', '5', '4']
        naipes = ['♠️', '♦️', '♥️', '♣️']
        self.deck = [f'{j}  {i}' for j in naipes for i in num]

    def get_hand(self, ROUND):
        hand = random.sample(self.deck, ROUND)
        return hand
    
    def draw_card(self):
        return random.choice(self.deck)
   
    def show_hand(self, player):
        print()
        for card in player.hand:
            print(card, '   ', end='')
        print('\n')
