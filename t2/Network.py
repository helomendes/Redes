import socket
import json
from Message import Message

class Network:
    def __init__(self):
        self.create_socket()
        self.ports = [[2000, 2001], [2001, 2002], [2002, 2003], [2003, 2000]]
        self.token = None
        self.players = {
                ('localhost', 2000): 1,
                ('localhost', 2001): 2,
                ('localhost', 2002): 3,
                ('localhost', 2003): 4
                }

    def get_local_ip(self):
        hostname = socket.gethostname()
        local_ip = socket.gethostbyname(hostname)
        return str(local_ip)

    def create_socket(self):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.settimeout(0.1)

    def bind(self, player):
        try:
            self.sock.bind(player.org_addr)
        except Exception as e:
            print(e)
            exit()

    def create_token(self, msg, player):
        if player.dealer and not self.token:
            self.token = msg.create_message(msg.token_type, False, player.id, player.id, 'token')
            player.token = True

    def pass_token(self, msg, player):
        msg.send_message(self, player, self.token)

    def receive_token(self, player, data):
        self.token = data
        player.token = True

    def establish_network(self, player, msg):
        packet_1 = msg.create_message(msg.test_type, True, player.id, player.id, 'establishing network')
        packet_2 = msg.create_message(msg.test_type, True, player.id, player.id, 'network established')

        if player.id == 1:
            received = False
            while not received:
                msg.send_message(self, player, packet_1)
                data = msg.receive_message(self)
                if msg.permission(player, data):
                    if data['data'] == packet_1['data']:
                        print('NETWORK ESTABLISHED')
                        print()
                        received = True
            msg.send_message(self, player, packet_2)
            while True:
                data = msg.receive_message(self)
                if msg.permission(player, data):
                    if data['data'] == packet_2['data']:
                        break   
        else:
            while True:
                data = msg.receive_message(self)
                if msg.permission(player, data):
                    if data['data'] == packet_1['data']:
                        break
            while True:
                msg.send_message(self, player, data)
                data_2 = msg.receive_message(self)
                if msg.permission(player, data_2):
                    if data_2 and data_2['data'] == packet_2['data']:
                        msg.send_message(self, player, data_2)
                        break
