from Network import Network
from Player import Player
from Game import Game
from Message import Message

def main():
    ntw = Network()
    player = Player(ntw.ports)
    game = Game()
    msg = Message()

    ntw.bind(player)
    ntw.establish_network(player, msg)
    game.start(ntw, player, msg)


#   definir o carteador (começa sendo o player 1)
#       começa sendo o player 1
#       depois é quem ganhar o jogo
#   começar o jogo

if __name__=='__main__':
    main()
