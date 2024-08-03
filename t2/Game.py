from Network import Message

class Game:
    def __init__(self): None

    def start(self, ntw, player, msg):
        ntw.create_token(player, msg)
        get_out = 3
        while get_out > 0:
            while player.token:
                print('sending message:')
                print(ntw.token)
                ntw.pass_token(player)
            while not player.token:
                print('listening')
                data = ntw.hear()
                print('received:', data)
                if data == ntw.token:
                    player.token = True
            get_out -= 1
