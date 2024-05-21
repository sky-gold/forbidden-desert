class AuthException(Exception):
    def __init__(self, message):
        super().__init__(message)

class InvalidToken(AuthException):
    pass

class ExpiredToken(AuthException):
    pass