import time
from Network import Message
from Cards import Cards

class Game:
    def __init__(self):
        self.ROUND = 5 
        self.dealt_card = None
        self.dealer = None
        self.plays = None
        self.cards = None
        self.guesses = None

    def new_round(self, ntw, player, msg):
        self.dealt_card = None
        self.dealer = None
        self.plays = None
        self.cards = None
        self.guesses = None
        self.winner = None

        if not player.dealer:
            while True:
                data = msg.receive_message(ntw)
                msg.send_message(ntw, player, data)
                if data and data['type'] == msg.start_type:
                    player.hand = None
                    break
        else:
            start_msg = msg.create_message(msg.start_type, True, player.id, player.id, 'new round started')
            while True:
                msg.send_message(ntw, player, start_msg)
                data = msg.receive_message(ntw)
                if msg.is_mine(player, data) and data['type'] == msg.start_type:
                    player.hand = None
                    break

    def guess_round(self, ntw, msg, player, ROUND):
        guess_msg = player.take_a_guess(msg, self, ROUND)
        player.guessed = guess_msg['data']
        end_msg = msg.create_message(msg.end_type, True, player.id, self.dealer, 'end of guess round')

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
            
            guesses = sorted(guesses, key=lambda guess: guess[0])
            self.guesses = guesses
            print('Palpites:\n')
            for guess in self.guesses:
                print('Jogador', guess[0], ':', guess[1])
            print()

    def end_of_round(self, ntw, player, msg):
        if not player.dealer:
            while True:
                data = msg.receive_message(ntw)
                msg.send_message(ntw, player, data)
                if data and data['type'] == msg.end_type:
                    break
        else:
            end_msg = msg.create_message(msg.end_type, True, player.id, player.id, 'end of round')
            while True:
                msg.send_message(ntw, player, end_msg)
                data = msg.receive_message(ntw)
                if msg.is_mine(player, data) and data['type'] == msg.end_type:
                    break

    def calc_points(self, ntw, player, cards):
        if player.dealer:
            dealt = cards.points(self.dealt_card)
            manilha = cards.manilha(dealt)
            winner = (0, 0)
            for play in self.plays:
                point = cards.points(play[1])
                if point[1] == manilha[1]:
                    if winner[1] == manilha[1]:
                        if point[0] > winner[0]:
                            winner = point
                        elif point[0] == winner[0]:
                            self.winner = self.dealer
                            continue
                    else:
                        winner = point
                else:
                    if point[1] > winner[1]:
                        winner = point
                    elif point[1] == winner[1]:
                        if point[0] > winner[0]:
                            winner = point
                        elif point[0] == winner[0]:
                            self.winner = self.dealer
                            continue
                if winner == point:
                    self.winner = play[0]
                
            print('Vencedor do round: Jogador', self.winner, '\n')

    def declare_winner(self, ntw, player):
        if player.dealer:
            winner = (0, 0)
            for life in self.lives:
                if life[1] > winner[1]:
                    winner = life
            self.winner = winner[0]

            print('VENCEDOR: Jogador', self.winner)


    def start(self, ntw, player, msg):
        ntw.create_token(msg, player)
        cards = Cards()

        player.i_am_dealer(ntw, msg, self, False)

        if not player.dealer:
            player.receive_cards(ntw, msg, self)
        else:
            player.deal_cards(ntw, msg, cards, self)
        print('Carta virada:', self.dealt_card)
        cards.show_hand(player)
        self.guess_round(ntw, msg, player, self.ROUND)
        
        for ROUND in range(self.ROUND, 0, -1):

            print('----------------------------------------')
            print('\nROUND', self.ROUND-ROUND+1, end='')

            player.i_am_dealer(ntw, msg, self, True)
            print('    -    Carteador: Jogador', self.dealer, '\n')

            cards.show_hand(player)
            player.play(ntw, msg, self)
            self.calc_points(ntw, player, cards)

            while True:
                if not player.dealer:
                    received = False
                    while not received:
                        data = msg.receive_message(ntw)
                        if data and data['type'] == msg.token_type:
                            received = True
                            if msg.is_for_me(player, data):
                                ntw.token = data
                                player.dealer = True
                        msg.send_message(ntw, player, data)
                    break
                else:
                    ntw.token['destination'] = self.winner 
                    ntw.pass_token(msg, player)
                    player.dealer = False
            print('----------------------------------------')

        player.i_am_dealer(ntw, msg, self, False)
        dif = player.guessed - player.wins
        if dif < 0:
            dif = dif * -1
        player.life -= dif
        player.show_life(ntw, msg, self)
        self.declare_winner(ntw, player)
