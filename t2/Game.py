import time
from Network import Message
from Cards import Cards

class Game:
    def __init__(self):
        self.dealt_card = None

    def token_tryout(self, ntw, player, msg):
        ntw.create_token(msg, player)
        while True:
            while not player.token:
                data = msg.receive_message(ntw)
                if msg.permission(player, data):
                    if data['type'] == msg.token_type:
                        print('Token received')
                        ntw.receive_token(player, data)

            while player.token:
                print('Token passed')
                ntw.token['origin'] = player.org_addr
                ntw.token['destination'] = player.dest_addr
                ntw.pass_token(msg, player)

    def new_round(self, ntw, player, msg, game):
        game.dealt_card = None
        game.dealer = None
        game.guesses = None
        game.cards = None

        if not player.dealer:
            while True:
                data = msg.receive_message(ntw)
                msg.send_message(ntw, player, data)
                if data and data['type'] == msg.start_type:
                    player.hand = None
                    break
        else:
            start_msg = msg.create_message(msg.start_type, True, player.org_addr, player.org_addr, 'new round started')
            while True:
                msg.send_message(ntw, player, start_msg)
                data = msg.receive_message(ntw)
                if msg.is_mine(player, data) and data['type'] == msg.start_type:
                    player.hand = None
                    break

    def guess_round(self, ntw, msg, player, game, ROUND):
        guess_msg = player.take_a_guess(msg, game, ROUND)
        '''
        guesses = []
        while len(guesses) < 4:
            msg.send_message(ntw, player, guess_msg)
            data = msg.receive_message(ntw)
            msg.send_message(ntw, player, data)
            if msg.is_for_me(player, data) and data['type'] == msg.guess_type:
                guess = (data['origin'], data['data'])
                if guess not in guesses:
                    guesses.append(guess)
            elif data and data['type'] == msg.warning_type:
                break
        if player.dealer:
            end_msg = msg.create_message(msg.warning_type, True, player.org_addr, player.org_addr, 'end of guesses')
            while True:
                msg.send_message(ntw, player, end_msg)
                data = msg.receive_message(ntw)
                if msg.is_mine(player, data):
                    break
                msg.send_message(ntw, player, data)

        
        '''
        if not player.dealer:
            while True:
                msg.send_message(ntw, player, guess_msg)
                data = msg.receive_message(ntw)
                msg.send_message(ntw, player, data)
                if data and data['type'] == msg.warning_type:
                    break
        else:
            guesses = []
            guess = (guess_msg['origin'], guess_msg['data'])
            guesses.append(guess)
            while len(guesses) < 4:
                data = msg.receive_message(ntw)
                msg.send_message(ntw, player, data)
                if data and data['type'] == msg.guess_type:
                    guess = (data['origin'], data['data'])
                    if guess not in guesses:
                        guesses.append(guess)
            end_msg = msg.create_message(msg.warning_type, True, player.org_addr, player.org_addr, 'end of guesses')
            while True:
                msg.send_message(ntw, player, end_msg)
                data = msg.receive_message(ntw)
                if msg.is_mine(player, data) and data['data'] == end_msg['data']:
                    break
                msg.send_message(ntw, player, data)

            guesses = sorted(guesses, key=lambda guess: guess[0][1])
            game.guesses = guesses
            for guess in guesses:
                print('Player', ntw.players[guess[0]], ':', guess[1])
            print()
        

    def end_of_round(self, ntw, player, msg):
        if not player.dealer:
            while True:
                data = msg.receive_message(ntw)
                msg.send_message(ntw, player, data)
                if data and data['type'] == msg.end_type:
                    break
        else:
            end_msg = msg.create_message(msg.end_type, True, player.org_addr, player.org_addr, 'end of round')
            while True:
                msg.send_message(ntw, player, end_msg)
                data = msg.receive_message(ntw)
                if msg.is_mine(player, data) and data['type'] == msg.end_type:
                    break

    def start(self, ntw, player, msg):
        ntw.create_token(msg, player)
        cards = Cards()

        for ROUND in range(4, 0, -1):

            self.new_round(ntw, player, msg, self)

            print('----------------------------------------')
            print('\nROUND', 4-ROUND+1, end='')

            player.i_am_dealer(ntw, msg, self)

            if not player.dealer:
                player.receive_cards(ntw, msg, self)
            else:
                player.deal_cards(ntw, msg, cards, self, ROUND)
            print('Dealt card:', self.dealt_card)
            cards.show_hand(player)

            self.guess_round(ntw, msg, player, self, ROUND)

            # cada um joga uma carta
    
            for _ in range(len(player.hand)):
                cards.show_hand(player)
                player.play(ntw, msg, self)
            # calculo de pontos
            # termina as cartas
            # define ganhador
            # o ganhador vira o dealer
            # o bastao é passado para o ganhador
            if player.dealer:
                print('calculo de pontos')
                print('definir ganhador')
                print('definir novo dealer')
                print('passa o bastao')
            print()
            
            self.end_of_round(ntw, player, msg)

            '''
            while player.token:
                print('Player', player.id, 'passing token')
                ntw.pass_token(msg, player)
                data = msg.receive_message(ntw)
                if data['type'] == msg.token_type:
                    player.token = False
            
            while not player.token:
                print('Player', player.id, 'receiving token')
                data = msg.receive_message(ntw)
                msg.send_message(ntw, player, data)
                if msg.permission(player, data) == 2:
                    if data['type'] == msg.token_type:
                        print('its for me')
                        ntw.receive_token(player, data)
                else:
                    break
            '''

