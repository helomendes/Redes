import random
from Cards import Cards

class Player:
    def __init__(self, ntw, ip_addr, game):
        self.id = int(input('ID do Jogador: '))
        self.set_addrs(ntw, ip_addr)
        self.set_dealer()
        self.hand = None
        self.life = game.ROUND
        self.wins = 0

    def set_addrs(self, ntw, ip_addr):
        ip_local = ntw.get_local_ip()
        self.org_addr = (ip_local, ntw.ports[self.id-1][0])
        self.dest_addr = (f'{ip_addr}', ntw.ports[self.id-1][1])


    def set_dealer(self):
        if self.id == 1:
            self.dealer = True
        else:
            self.dealer = False

    def i_am_dealer(self, ntw, msg, game, WINNER):
        if self.dealer:
            game.dealer = self.id
            dealer_msg = msg.create_message(msg.dealer_type, True, self.id, self.id, 'I am the dealer')
            if WINNER:
                self.wins += 1
            while True:
                msg.send_message(ntw, self, dealer_msg)
                data = msg.receive_message(ntw)
                if msg.is_mine(self, data) and data['data'] == dealer_msg['data']:
                    break
        else:
            while True:
                data = msg.receive_message(ntw)
                msg.send_message(ntw, self, data)
                if data and data['broadcast'] and data['type'] == msg.dealer_type:
                    game.dealer = data['origin']
                    break
        
    def deal_cards(self, ntw, msg, cards, game):
        for PLAYER in range(4):
            hand = cards.get_hand(game.ROUND)
            if self.id-1 == PLAYER:
                self.hand = hand
            else:
                hand_msg = msg.create_message(msg.hand_type, False, self.id, PLAYER+1, hand)

                while True:
                    msg.send_message(ntw, self, hand_msg)
                    data = msg.receive_message(ntw)
                    if msg.is_mine(self, data) and data['data'] == hand_msg['data']:
                        break

        game.dealt_card = cards.draw_card()
        card_msg = msg.create_message(msg.card_type, True, self.id, self.id, game.dealt_card)

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
        while True:
            guess = int(input('Dê seu palpite: '))
            if guess >= 0 and guess < game.ROUND:
                break
            print('Inválido. Tente novamente.')
        
        #guess = random.randint(0, ROUND)
        print('Meu palpite:', guess, '\n')

        guess_msg = msg.create_message(msg.guess_type, False, self.id, game.dealer, guess)
        return guess_msg

    def pick_a_card(self, msg, game):
        while True:
            ind = int(input('Escolha uma carta: '))
            if ind > 0 and ind <= len(self.hand):
                break
            print('Inválido. Tente novamente.')

        card = self.hand[ind-1]
        
        #card = random.choice(self.hand)
        print('Carta jogada: ', card, '\n')
        self.hand.remove(card)
        card_msg = msg.create_message(msg.play_type, False, self.id, game.dealer, card)

        return card_msg

    def play(self, ntw, msg, game):
        play_msg = self.pick_a_card(msg, game)
        end_msg = msg.create_message(msg.end_type, True, self.id, game.dealer, 'end of play round')
        
        if not self.dealer:
            received = False
            while not received:
                data = msg.receive_message(ntw)
                if data and data['type'] == msg.play_type:
                    received = True
            received = False
            while not received:
                if data and data['type'] == msg.play_type:
                    msg.send_message(ntw, self, data)
                msg.send_message(ntw, self, play_msg)
                data = msg.receive_message(ntw)
                if msg.is_mine(self, data) and data['data'] == play_msg['data']:
                    received = True
            received = False
            while not received:
                data = msg.receive_message(ntw)
                if data and data['data'] == end_msg['data']:
                    received = True
                msg.send_message(ntw, self, data)


        else:
            plays = []
            while len(plays) < 4:
                msg.send_message(ntw, self, play_msg)
                data = msg.receive_message(ntw)
                if msg.is_for_me(self, data) and data['type'] == msg.play_type:
                    play = (data['origin'], data['data'])
                    if play not in plays:
                        plays.append(play)
                msg.send_message(ntw, self, data)

            received = False
            while not received:
                msg.send_message(ntw, self, end_msg)
                data = msg.receive_message(ntw)
                if data and data['data'] == end_msg['data']:
                    received = True

            plays = sorted(plays, key=lambda play: play[0])
            game.plays = plays
            print('Cartas jogadas:\n')
            for play in game.plays:
                print('Jogador', play[0], ':', play[1])
            print()

    def show_life(self, ntw, msg, game):
        life_msg = msg.create_message(msg.life_type, False, self.id, game.dealer, self.life)
        end_msg = msg.create_message(msg.end_type, True, self.id, game.dealer, 'end of life round')

        if not self.dealer:
            received = False
            while not received:
                data = msg.receive_message(ntw)
                if data and data['type'] == msg.life_type:
                    received = True
            received = False
            while not received:
                if data and data['type'] == msg.life_type:
                    msg.send_message(ntw, self, data)
                msg.send_message(ntw, self, life_msg)
                data = msg.receive_message(ntw)
                if msg.is_mine(self, data) and data['data'] == life_msg['data']:
                    received = True
            received = False
            while not received:
                data = msg.receive_message(ntw)
                if data and data['data'] == end_msg['data']:
                    received = True
                msg.send_message(ntw, self, data)

        else:
            lives = []
            while len(lives) < 4:
                msg.send_message(ntw, self, life_msg)
                data = msg.receive_message(ntw)
                if msg.is_for_me(self, data) and data['type'] == msg.life_type:
                    life = (data['origin'], data['data'])
                    if life not in lives:
                        lives.append(life)
                msg.send_message(ntw, self, data)

            received = False
            while not received:
                msg.send_message(ntw, self, end_msg)
                data = msg.receive_message(ntw)
                if data and data['data'] == end_msg['data']:
                    received = True

            lives = sorted(lives, key=lambda life: life[0])
            game.lives = lives
            print('Vidas:\n')
            for life in game.lives:
                print('Player', life[0], ':', life[1])
            print()

