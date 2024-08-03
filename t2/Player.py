class Player:
    def __init__(self, ports):
        self.id = int(input())
        self.org_addr = ('localhost', ports[self.id-1][0])
        self.dest_addr = ('localhost', ports[self.id-1][1])
        self.set_dealer()
        self.token = False
        self.msg_queue = []
        self.life = 12

    def set_dealer(self):
        if self.id == 1:
            self.dealer = True
        else:
            self.dealer = False

    def add_message(self, msg):
        self.msg_queue.append(msg)

