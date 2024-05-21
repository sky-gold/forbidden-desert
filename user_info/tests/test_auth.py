import sys
import os

project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
sys.path.append(project_root)

from fastapi.testclient import TestClient
from unittest.mock import Mock
from sqlalchemy.orm import Session
import pytest
import json
import hashlib
import hmac
import time

from main import app
import auth

client = TestClient(app)

def get_token(bot_token, auth_date = time.time(), id = 123, name = 'Vasya'):
    data = {
        "id": id,
        "first_name": name,
        "last_name": "Pupkin",
        "username": "pupkin",
        "photo_url": "https://placehold.co/500x500",
        "auth_date": auth_date
    }
    a = []
    for key, value in data.items():
        if key == "hash":
            continue
        a.append(f"{key}={value}")
    a.sort()
    data_check_string = '\n'.join(a)
    secret_key = hashlib.sha256(bot_token.encode()).digest()
    check_hash = hmac.new(secret_key, msg=data_check_string.encode(), digestmod=hashlib.sha256).hexdigest()
    data['hash'] = check_hash
    return json.dumps(data)
    


@pytest.mark.parametrize("token, expected_detail", [
    ("invalid", "Token is invalid"),
    (get_token("mock_bot_token", time.time() - 10 * (24 * 60 * 60)), "Token has expired"),
], ids=list(range(1, 3)))
def test_auth_401(mocker, token, expected_detail):
    mocker.patch.object(auth, 'BOT_TOKEN', 'mock_bot_token')

    response = client.get("/auth", headers={"Authorization": token})

    assert response.status_code == 401
    assert response.json()['detail'] == expected_detail

def test_auth_existing_user_200(mocker):
    token = get_token("mock_bot_token", time.time(), 1, 'telegram_name')
    mocker.patch.object(auth, 'BOT_TOKEN', 'mock_bot_token')
    mock_execute = mocker.patch.object(Session, 'execute', return_value=Mock(fetchone=lambda: (1, 'db_name', 'reg_date')))
    response = client.get("/auth", headers={"Authorization": token})
    print("response.json() =", response.json())
    assert response.status_code == 200
    assert response.json()['id'] == 1
    assert response.json()['name'] == 'db_name'
    assert response.json()['registration_date'] == 'reg_date'

def test_auth_new_user_200(mocker):
    token = get_token("mock_bot_token", time.time(), 1, 'telegram_name')
    mocker.patch.object(auth, 'BOT_TOKEN', 'mock_bot_token')
    mock_execute = mocker.patch.object(Session, 'execute')
    mock_execute.return_value = Mock(fetchone=lambda: None if mock_execute.call_count == 1 else (1, 'telegram_name', 'reg_date'))
    mock_commit = mocker.patch.object(Session, 'commit')
    mock_rollback = mocker.patch.object(Session, 'rollback')
    response = client.get("/auth", headers={"Authorization": token})
    print("response.json() =", response.json())
    assert response.status_code == 200

    assert mock_execute.call_count == 3

    assert str(mock_execute.call_args_list[0][0][0]) == "SELECT id, name, registration_date FROM users WHERE id = :user_id"
    assert mock_execute.call_args_list[0][0][1] == {'user_id': 1}

    assert str(mock_execute.call_args_list[1][0][0]) == "INSERT INTO users (id, name) VALUES (:user_id, :name)"
    assert mock_execute.call_args_list[1][0][1] == {"user_id": 1, "name": 'telegram_name'}

    assert str(mock_execute.call_args_list[2][0][0]) == "SELECT id, name, registration_date FROM users WHERE id = :user_id"
    assert mock_execute.call_args_list[2][0][1] == {'user_id': 1}

    mock_commit.assert_called_once()
    mock_rollback.assert_not_called()
    assert response.json()['id'] == 1
    assert response.json()['name'] == 'telegram_name'
    assert response.json()['registration_date'] == 'reg_date'
