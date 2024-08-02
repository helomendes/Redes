import socket

class Player:
    def __init__(self, ports):
        self.id = int(input())
        self.org_addr = ('localhost', ports[self.id-1][0])
        self.dest_addr = ('localhost', ports[self.id-1][1])
        self.dealer = False
        self.token = False
        self.msg_queue = []
        self.msg_queue.append('primeira mensagem')
        self.msg_queue.append('segunda mensagem')
        self.msg_queue.append('terceira mensagem')
 
class Conexao:
    def __init__(self):
        self.ports = [[2000, 2001], [2001, 2002], [2002, 2003], [2003, 2000]]
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.settimeout(0.1)
        self.token = 'bastao'

    def criar_bastao(self, player):
        if player.dealer:
            player.token = True

    def passar_bastao(self, player):
        self.sock.sendto(self.token.encode(), player.dest_addr)
        player.token = False


    def estabelecer_conexao(self, player):
        packet_1 = 'iniciando conexao'
        packet_2 = 'conexao estabelecida'
        confirma = 'confirmacao'

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
            self.sock.sendto(packet_2.encode(), player.dest_addr)
   
        else:
            while True:
                try:
                    data, _ = self.sock.recvfrom(1024)
                except Exception as e:
                    continue
                print('mensagem recebida:', data.decode())
                if data.decode() == packet_1:
                    break
            while True:
                self.sock.sendto(data, player.dest_addr)
                try:
                    data_2, _ = self.sock.recvfrom(1024)
                except Exception as e:
                    continue
                print('mensagem recebida:', data_2.decode())
                if data_2.decode() == packet_2:
                    self.sock.sendto(data_2, player.dest_addr)
                    break

    def estruturando_rede(self, player):
        self.criar_bastao(player)
        while True:
            while player.token:
                print('mandando mensagem')
                self.passar_bastao(player)
            while not player.token:
                print('ouvindo')
                try:
                    data, _ = self.sock.recvfrom(1024)
                except Exception as e:
                    continue
                if data.decode() == self.token:
                    player.token = True

def main():
    cnx = Conexao()
    player = Player(cnx.ports)
    try:
        cnx.sock.bind(player.org_addr)
    except Exception as e:
        print(e)
        exit()
    if player.id == 1:
        player.dealer = True
    cnx.estabelecer_conexao(player)
    cnx.estruturando_rede(player)

# definir o carteador (começa sendo o player 1)
# começar o jogo 

if __name__=='__main__':
    main()
