import socket
import json

class Message:
    def __init__(self):
        self.token_type = 'token_type'

    def create_message(self, typ, broadcast, org, dest, data):
        msg = {
                "type": {typ},
                "broadcast": {broadcast},
                "origem": {org},
                "destination": {dest},
                "data": {data}
        }
        return msg

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

    def create_token(self, player):
        if player.dealer:
            player.token = True


    def pass_token(self, player):
        self.speak(self.token, player.dest_addr)
        player.token = False

    def hear(self):
        try:
            data, _ = self.sock.recvfrom(1024)
            return data.decode()
        except Exception as e:
            pass

    def speak(self, msg, addr):
        if type(msg) == str:
            msg = msg.encode()
        self.sock.sendto(msg, addr)

    def establish_network(self, player):
        packet_1 = 'establishing network'
        packet_2 = 'network established'

        if player.id == 1:
            received = False
            while not received:
                self.speak(packet_1, player.dest_addr)
                data = self.hear()
                if data == packet_1:
                    print('network established')
                    received = True
            self.speak(packet_2, player.dest_addr)
            while True:
                data = self.hear()
                if data == packet_2:
                    break   
        else:
            while True:
                data = self.hear()
                print('message received:', data)
                if data == packet_1:
                    break
            while True:
                self.speak(data, player.dest_addr)
                data_2 = self.hear()
                print('message received:', data_2)
                if data_2 == packet_2:
                    self.speak(data_2, player.dest_addr)
                    break
