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
        for i, card in enumerate(player.hand):
            print(i+1, ':', card, '   ', end='')
        print('\n')

    def points(self, card):
        naipe = 0
        num = 0

        if card[0] == '♣️' or card[0] == '♣':
            naipe = 4
        elif card[0] == '♥️' or card[0] == '♥':
            naipe = 3
        elif card[0] == '♠️' or card[0] == '♠':
            naipe = 2
        elif card[0] == '♦️' or card[0] == '♦':
            naipe = 1

        if card[4] == '3':
            num = 10
        elif card[4] == '2':
            num = 9
        elif card[4] == 'A':
            num = 8
        elif card[4] == 'K':
            num = 7
        elif card[4] == 'J':
            num = 6
        elif card[4] == 'Q':
            num = 5
        elif card[4] == '7':
            num = 4
        elif card[4] == '6':
            num = 3
        elif card[4] == '5':
            num = 2
        elif card[4] == '4':
            num = 1

        return (naipe, num)

    def manilha(self, card):
        if card[1] == 10:
            return (0, 1)
        else:
            return (0, card[1]+1)
