import sys
from Network import Network
from Player import Player
from Game import Game
from Message import Message

def main():
    ip_addr = sys.argv[1]
    ntw = Network()
    game = Game()
    player = Player(ntw, ip_addr, game)
    msg = Message()

    ntw.bind(player)
    ntw.establish_network(player, msg)
    game.start(ntw, player, msg)

if __name__=='__main__':
    main()
