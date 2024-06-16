import json
import hashlib
import hmac
import time
import os

from exceptions import InvalidToken, ExpiredToken

BOT_TOKEN = os.getenv("BOT_TOKEN")

class HashCheck:
    def __init__(self, data, secret):
        self.hash = data['hash']
        self.secret_key = hashlib.sha256(secret).digest()
        print("secret_key=", self.secret_key)
        self.data = {}
        for k, v in data.items():
            if k != 'hash':
                self.data[k] = str(v)

    def data_check_string(self):
        a = sorted(self.data.items())
        res = '\n'.join(map(lambda x: '='.join(x), a))
        return res

    def calc_hash(self):
        msg = bytearray(self.data_check_string(), 'utf-8')
        res = hmac.new(self.secret_key, msg=msg, digestmod=hashlib.sha256).hexdigest()
        return res
    
    def check_hash(self):
        return self.calc_hash() == self.hash

# def check_auth_data(data):
#     print("BOT_TOKEN=", BOT_TOKEN)
#     d = data.copy()
#     del d['hash']
#     d_list = []
#     for key in sorted(d.keys()):
#         if d[key] != None:
#             d_list.append(f"{key}={d[key]}")
#     data_string = bytes('\n'.join(d_list), 'utf-8')
#     print("data_string =", data_string)
#     secret_key = hashlib.sha256(BOT_TOKEN.encode('utf-8')).digest()
#     print("secret_key =", secret_key)
#     hmac_string = hmac.new(secret_key, data_string, hashlib.sha256).hexdigest()
#     if hmac_string == data['hash']:
#         return True
#     return False

def check_auth(token: str):
    print("check_auth")
    print("token=", token)
    token = token.encode('iso-8859-1').decode('utf-8')
    print("new_token=", token)
    try:
        data = json.loads(token)
    except:
        raise InvalidToken("Token is invalid")
    if not isinstance(data, dict):
        raise InvalidToken("Token is invalid")
    if "hash" not in data:
        raise InvalidToken("Token is invalid")
    
    hash_check = HashCheck(data, BOT_TOKEN.encode('utf-8'))
    print("data_check_string =", hash_check.data_check_string())
    print("calc_hash =", hash_check.calc_hash())
    if not hash_check.check_hash():
        raise InvalidToken("Token hash is invalid");
    cur_time = time.time()

    if (abs(data['auth_date'] - cur_time) > 3 * (24 * 60 * 60)):
        raise ExpiredToken("Token has expired")
    
    return {'id': int(data['id']), 'name': data['first_name']}