from Network import Message
from Cards import Cards

class Game:
    def __init__(self):
        self.dealt_card = None

    def token_tryout(self, ntw, player, msg):
        while True:
            while not player.token:
                data = msg.receive_message(ntw)
                if msg.permission(player, data):
                    if data['type'] == msg.token_type:
                        print('Recebi o bastao')
                        ntw.receive_token(player, data)

            while player.token:
                print('Passando o bastao')
                ntw.pass_token(msg, player)

    def deal_cards(self, ntw, msg, player, cards, ROUND):
        for PLAYER in range(4):
            hand = cards.get_hand(ROUND)
            if player.id-1 == PLAYER:
                player.hand = hand
                print(player.hand)
            else:
                hand_msg = msg.create_message(msg.hand_type, False, player.org_addr, ('localhost', ntw.ports[PLAYER][0]), hand)
                if player.token:
                    received = False
                    while not received:
                        msg.send_message(ntw, player, hand_msg)
                        data = msg.receive_message(ntw)
                        if data and data['data'] == hand_msg['data']:
                            received = True
                else:
                    player.msg_queue.append(hand_msg)

        self.dealt_card = cards.draw_card()
        card_msg = msg.create_message(msg.card_type, True, player.org_addr, player.org_addr, self.dealt_card)
        
        while True:
            msg.send_message(ntw, player, card_msg)
            data = msg.receive_message(ntw)
            if msg.permission(player, data):
                if data['data'] == card_msg['data']:
                    break
        

    def receive_cards(self, ntw, msg, player):
        while True:
            data = msg.receive_message(ntw)
            msg.send_message(ntw, player, data)
            if msg.permission(player, data):
                if data['destination'] == player.org_addr:
                    player.hand = data['data']
                    print(player.hand)
                else:
                    self.dealt_card = data['data'] 
                    break

    def start(self, ntw, player, msg):
        ntw.create_token(msg, player)
        cards = Cards()

        for ROUND in range(4, 0, -1):
            if player.dealer:
                self.deal_cards(ntw, msg, player, cards, ROUND)
            else:
               self.receive_cards(ntw, msg, player)
            print('Dealt card:', self.dealt_card)




