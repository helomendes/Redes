from Network import Network
from Player import Player
from Game import Game

def main():
    ntw = Network()
    player = Player(ntw.ports)
    game = Game()

    ntw.bind(player)
    ntw.establish_network(player)
    #game.start(ntw, player)


#   definir o carteador (começa sendo o player 1)
#       começa sendo o player 1
#       depois é quem ganhar o jogo
#   começar o jogo

if __name__=='__main__':
    main()
