import json

class Message:
    def __init__(self):
        self.token_type = 'token_type'
        self.test_type = 'test_type'
        self.start_type = 'start_type'
        self.end_type = 'end_type'
        self.hand_type = 'hand_type'
        self.card_type = 'card_type'
        self.guess_type = 'guess_type'
        self.warning_type = 'warning_type'
        self.dealer_type = 'dealer_type'

    def create_message(self, typ, broadcast, org, dest, data):
        msg = {
                "type": typ,
                "broadcast": broadcast,
                "origin": org,
                "destination": dest,
                "data": data
        }
        return msg
    
    def send_message(self, ntw, player, data):
        data = json.dumps(data)
        ntw.sock.sendto(data.encode(), player.dest_addr)

    def receive_message(self, ntw):
        try:
            data, _ = ntw.sock.recvfrom(1024)
            data = json.loads(data.decode())
            data['origin'] = (data['origin'][0], data['origin'][1])
            data['destination'] = (data['destination'][0], data['destination'][1])
            return data
        except Exception as e:
            return None

    def permission(self, player, data):
        if data:
            if data['broadcast']:
                return 1
            elif data['destination'] == player.org_addr:
                return 2
        return 0

    def is_for_me(self, player, data):
        if data:
            if data['destination'] == player.org_addr:
                return True
        return False

    def is_mine(self, player, data):
        if data:
            if data['origin'] == player.org_addr:
                return True
        return False

    '''
    def send_until_receive(self, ntw, player, data):
        while True:
            self.send_message(ntw, player, data)
            rec = self.receive_message(ntw)
            if self.is_mine(player, rec) and data['data'] == rec['data']:
                    break
            self.send_message(ntw, player, rec)
    '''
