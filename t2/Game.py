import time
from Network import Message
from Cards import Cards

class Game:
    def __init__(self):
        self.ROUND = 4
        self.dealt_card = None
        self.dealer = None
        self.plays = None
        self.cards = None
        self.guesses = None

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

    def new_round(self, ntw, player, msg):
        self.dealt_card = None
        self.dealer = None
        self.plays = None
        self.cards = None
        self.guesses = None

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

    def guess_round(self, ntw, msg, player, ROUND):
        guess_msg = player.take_a_guess(msg, self, ROUND)
        end_msg = msg.create_message(msg.end_type, True, player.org_addr, self.dealer, 'end of guess round')

        if not player.dealer:
            received = False
            while not received:
                data = msg.receive_message(ntw)
                if data and data['type'] == msg.guess_type:
                    received = True
            received = False
            while not received:
                msg.send_message(ntw, player, data)
                msg.send_message(ntw, player, guess_msg)
                data = msg.receive_message(ntw)
                if msg.is_mine(player, data) and data['data'] == guess_msg['data']:
                    received = True
                elif data and data['data'] == end_msg['data']:
                    received = True
                msg.send_message(ntw, player, data)
            received = False
            while not received:
                data = msg.receive_message(ntw)
                if data and data['data'] == end_msg['data']:
                    received = True
                msg.send_message(ntw, player, data)
        else:
            guesses = []
            while len(guesses) < 4:
                msg.send_message(ntw, player, guess_msg)
                data = msg.receive_message(ntw)
                if msg.is_for_me(player, data) and data['type'] == msg.guess_type:
                    guess = (data['origin'], data['data'])
                    if guess not in guesses:
                        guesses.append(guess)
                msg.send_message(ntw, player, data)
                
            received = False
            while not received:
                msg.send_message(ntw, player, end_msg)
                data = msg.receive_message(ntw)
                if data and data['data'] == end_msg['data']:
                    received = True
            
            guesses = sorted(guesses, key=lambda guess: guess[0][1])
            self.guesses = guesses
            for guess in self.guesses:
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

    def calc_points(self, player):
        if player.dealer:
            print('***************************************')
            print('Dealt card:', self.dealt_card)
            print('Plays:')
            print(self.plays)



        return

    def start(self, ntw, player, msg):
        ntw.create_token(msg, player)
        cards = Cards()

        player.i_am_dealer(ntw, msg, self)

        if not player.dealer:
            player.receive_cards(ntw, msg, self)
        else:
            player.deal_cards(ntw, msg, cards, self)
        print('Dealt card:', self.dealt_card)
        cards.show_hand(player)

        for ROUND in range(self.ROUND, 0, -1):

            print('----------------------------------------')
            print('\nROUND', self.ROUND-ROUND+1, end='')

            player.i_am_dealer(ntw, msg, self)
            self.guess_round(ntw, msg, player, ROUND)

            cards.show_hand(player)
            player.play(ntw, msg, self)
            #self.calc_points(player)

            '''     
                if player.dealer:
                    while True:
                        data = msg.receive_message(ntw)
                        print(data)
                        msg.send_message(ntw, player, data)
                
                self.calc_points(player)
            
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
           
            '''
            #self.end_of_round(ntw, player, msg)
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

