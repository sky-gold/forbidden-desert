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

from test_auth import get_token

from main import app
import auth

client = TestClient(app)


def test_change_name_401(mocker):
    mocker.patch.object(auth, 'BOT_TOKEN', 'mock_bot_token')

    response = client.post(
        "/change/name",
        json={"new_name": "NewName"},
        headers={"Authorization": "invalid"}
    )

    print("response.json() =", response.json())
    assert response.status_code == 401
    assert response.json()['detail'] == "Token is invalid"


def test_change_name_200(mocker):
    token = get_token("mock_bot_token", time.time(), 1, 'telegram_name')
    mocker.patch.object(auth, 'BOT_TOKEN', 'mock_bot_token')
    mock_execute = mocker.patch.object(Session, 'execute')
    mock_execute.return_value = Mock(fetchone=lambda: None if mock_execute.call_count == 0 else (1, 'telegram_name', 'reg_date'))
    mock_commit = mocker.patch.object(Session, 'commit')
    mock_rollback = mocker.patch.object(Session, 'rollback')

    response = client.post(
        "/change/name",
        json={"new_name": "NewName"},
        headers={"Authorization": token}
    )
    
    print("response.json() =", response.json())

    assert response.status_code == 200

    assert mock_execute.call_count == 2

    assert str(mock_execute.call_args_list[0][0][0]) == "SELECT id, name, registration_date FROM users WHERE id = :user_id"
    assert mock_execute.call_args_list[0][0][1] == {'user_id': 1}

    assert str(mock_execute.call_args_list[1][0][0]) == "UPDATE users SET name = :new_name WHERE id = :user_id"
    assert mock_execute.call_args_list[1][0][1] == {"new_name": "NewName", "user_id": 1}

    mock_commit.assert_called_once()
    mock_rollback.assert_not_called()
