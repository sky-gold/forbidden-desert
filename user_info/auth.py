import json
import hashlib
import hmac
import time
import os

from exceptions import InvalidToken, ExpiredToken

BOT_TOKEN = os.getenv("BOT_TOKEN")

def check_auth(token: str):
    print("BOT_TOKEN=", BOT_TOKEN)
    try:
        data = json.loads(token)
    except:
        raise InvalidToken("Token is invalid")
    if not isinstance(data, dict):
        raise InvalidToken("Token is invalid")
    if "hash" not in data:
        raise InvalidToken("Token is invalid")
    a = []
    for key, value in data.items():
        if key == "hash":
            continue
        a.append(f"{key}={value}")
    a.sort()
    data_check_string = '\n'.join(a)
    secret_key = hashlib.sha256(BOT_TOKEN.encode()).digest()
    check_hash = hmac.new(secret_key, msg=data_check_string.encode(), digestmod=hashlib.sha256).hexdigest()
    if data["hash"] != check_hash:
        raise InvalidToken("Token is invalid")
    
    cur_time = time.time()

    if (abs(data['auth_date'] - cur_time) > 3 * (24 * 60 * 60)):
        raise ExpiredToken("Token has expired")
    
    return {'id': int(data['id']), 'name': data['first_name']}