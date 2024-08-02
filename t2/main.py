import socket

class Player:
    def __init__(self, ports):
        self.id = int(input())
        self.org_addr = ('localhost', ports[self.id-1][0])
        self.dest_addr = ('localhost', ports[self.id-1][1])

class Conexao:
    def __init__(self):
        self.ports = [[2000, 2001], [2001, 2002], [2002, 2003], [2003, 2000]]
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.settimeout(0.1)

    def estabelecer_conexao(self, player):
        packet_1 = 'iniciando conexao'

        if player.id == 1:
            received = False
            while not received:
                self.sock.sendto(packet_1.encode(), player.dest_addr)
                try:
                    data, _ = self.sock.recvfrom(1024)
                except Exception as e:
                    continue
                if data.decode() == packet_1:
                    print('conexao estabelecida')
                    received = True
        else:
            while True:
                try:
                    data, _ = self.sock.recvfrom(1024)
                except Exception as e:
                    continue
                if data.decode() == packet_1:
                    print('mensagem recebida:', data.decode())
                    break
            while True:
                self.sock.sendto(packet_1.encode(), player.dest_addr)

    def teste_bastao(self, player):
        bastao = 'bastao'


def main():
    cnx = Conexao()
    player = Player(cnx.ports)
    try:
        cnx.sock.bind(player.org_addr)
    except Exception as e:
        print(e)
        exit()
    cnx.estabelecer_conexao(player)


# criar bastao
# definir o carteador (começa sendo o player 1)
# começar o jogo 

if __name__=='__main__':
    main()
