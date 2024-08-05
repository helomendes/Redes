import random
from Cards import Cards

class Player:
    def __init__(self, ports):
        self.id = int(input())
        self.org_addr = ('localhost', ports[self.id-1][0])
        self.dest_addr = ('localhost', ports[self.id-1][1])
        self.set_dealer()
        self.token = False
        self.msg_queue = []
        self.life = 12

    def set_dealer(self):
        if self.id == 1:
            self.dealer = True
        else:
            self.dealer = False

    def i_am_dealer(self, ntw, msg, game):
        if self.dealer:
            game.dealer = self.org_addr
            dealer_msg = msg.create_message(msg.dealer_type, True, self.org_addr, self.org_addr, 'I am the dealer')

            # REPETIDO
            while True:
                msg.send_message(ntw, self, dealer_msg)
                data = msg.receive_message(ntw)
                if msg.is_mine(self, data) and data['data'] == dealer_msg['data']:
                    break
        else:
            while True:
                data = msg.receive_message(ntw)
                msg.send_message(ntw, self, data)
                if data['broadcast'] and data['type'] == msg.dealer_type:
                    game.dealer = data['origin']
                    break

        print('   -   Dealer: Player', ntw.players[game.dealer], '\n')

    def deal_cards(self, ntw, msg, cards, game, ROUND):
        for PLAYER in range(4):
            hand = cards.get_hand(ROUND)
            if self.id-1 == PLAYER:
                self.hand = hand
            else:
                hand_msg = msg.create_message(msg.hand_type, False, self.org_addr, ('localhost', ntw.ports[PLAYER][0]), hand)

                # REPETIDO
                while True:
                    msg.send_message(ntw, self, hand_msg)
                    data = msg.receive_message(ntw)
                    if msg.is_mine(self, data) and data['data'] == hand_msg['data']:
                        break

        game.dealt_card = cards.draw_card()
        card_msg = msg.create_message(msg.card_type, True, self.org_addr, self.org_addr, game.dealt_card)

        # REPETIDO
        while True:
            msg.send_message(ntw, self, card_msg)
            data = msg.receive_message(ntw)
            if msg.is_mine(self, data) and data['data'] == card_msg['data']:
                break

    def receive_cards(self, ntw, msg, game):
        while True:
            data = msg.receive_message(ntw)
            msg.send_message(ntw, self, data)
            if msg.is_for_me(self, data) and data['type'] == msg.hand_type:
                self.hand = data['data']
            elif data and data['type'] == msg.card_type:
                game.dealt_card = data['data']
                break
        
    def take_a_guess(self, msg, game, ROUND):
        '''
        guess = int(input('My guess: '))
        '''
        guess = random.randint(0, ROUND)
        print('My guess:', guess)

        guess_msg = msg.create_message(msg.guess_type, False, self.org_addr, game.dealer, guess)
        return guess_msg

    def pick_a_card(self, msg, game):
        '''
        ind = int(input('Pick a card: '))
        card = self.hand[ind-1]
        '''
        card = random.choice(self.hand)
        print(card)
        self.hand.remove(card)
        card_msg = msg.create_message(msg.card_type, False, self.org_addr, game.dealer, card)

        return card_msg

    def play(self, ntw, msg, game):
        card_msg = self.pick_a_card(msg, game)
        
        while len(game.cards) < 4:
            msg.send_message(ntw, self, card_msg)
            data = msg.receive_message(ntw)
            msg.send_message(ntw, self, data)
            if msg.is_for_me(self, data) and data['type'] == msg.card_type:
                card = (data['origin'], data['data'])
                if card not in game.cards:
                    game.cards.append(card)
            elif data and data['type'] == msg.warning_type:
                break

        if player.dealer:
            end_msg = msg.create_message()
            while True:
                msg.send_message()
                data = msg.receive_message()
                if is_mine:
                    break
                msg.send_message()
        

