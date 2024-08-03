import socket
import json
from Message import Message

class Network:
    def __init__(self):
        self.create_socket()
        self.ports = [[2000, 2001], [2001, 2002], [2002, 2003], [2003, 2000]]

    def create_socket(self):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.settimeout(0.1)

    def bind(self, player):
        try:
            self.sock.bind(player.org_addr)
        except Exception as e:
            print(e)
            exit()

    def create_token(self, player, msg):
        if player.dealer:
            player.token = True
        self.token = msg.create_message(msg.token_type, False, player.org_addr, player.dest_addr, 'token')

    def pass_token(self, player):
        self.speak(self.token)
        player.token = False

    def hear(self):
        try:
            data, _ = self.sock.recvfrom(1024)
            return json.loads(data.decode())
        except Exception as e:
            pass

    def speak(self, msg):
        msg_str = json.dumps(msg)
        self.sock.sendto(msg_str.encode(), msg['destination'])

    def establish_network(self, player, msg):
        packet_1 = msg.create_message(msg.test_type, True, player.org_addr, player.dest_addr, 'establishing network')
        packet_2 = msg.create_message(msg.test_type, True, player.org_addr, player.dest_addr, 'network established')
        print(type(player.dest_addr))

        if player.id == 1:
            received = False
            while not received:
                self.speak(packet_1)
                data = self.hear()
                if data == packet_1:
                    print('network established')
                    received = True
            self.speak(packet_2)
            while True:
                data = self.hear()
                if data == packet_2:
                    break   
        else:
            while True:
                data = self.hear()
                print('message received:', data)
                print(type(data['destination']), data['destination'])
                if data == packet_1:
                    break
            while True:
                self.speak(data)
                data_2 = self.hear()
                print('message received:', data_2)
                if data_2 == packet_2:
                    self.speak(data_2)
                    break
