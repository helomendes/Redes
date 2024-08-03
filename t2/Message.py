import json

class Message:
    def __init__(self):
        self.token_type = 'token_type'
        self.test_type = 'test_type'
        self.hand_type = 'hand_type'
        self.card_type = 'card_type'

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
                return True
            elif data['destination'] == player.org_addr:
                return True
        return False
