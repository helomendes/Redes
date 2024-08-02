from Network import Message

class Game:
    def __init__(self): None

    def start(self, ntw, player):
        msg = Message()
        ntw.create_token(player, msg)
        while True:
            while player.token:
                print('sending message')
                ntw.pass_token(player)
            while not player.token:
                print('listening')
                data = ntw.hear()
                if data == ntw.token:
                    player.token = True
