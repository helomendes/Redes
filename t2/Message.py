class Message:
    def __init__(self):
        self.token_type = 'token_type'
        self.test_type = 'test_type'

    def create_message(self, typ, broadcast, org, dest, data):
        msg = {
                "type": typ,
                "broadcast": broadcast,
                "origem": org,
                "destination": dest,
                "data": data
        }
        return msg
    
    def receive_message(self, ntw):
        msg = ntw.hear()
