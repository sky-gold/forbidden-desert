from fastapi import FastAPI, Depends, HTTPException, Header


from auth import check_auth
from exceptions import InvalidToken, ExpiredToken
from database import get_user, add_user


app = FastAPI()

async def get_authorization_header(authorization: str = Header(...)):
    return authorization

@app.get("/auth")
async def auth(token: str = Depends(get_authorization_header)):
    print("token:", token)
    try:
        user_data = check_auth(token)
    except (InvalidToken, ExpiredToken) as e:
        raise HTTPException(status_code=401, detail=str(e))
    try:    
        user = get_user(user_data["id"])
        print("!!!user=", user)
        if not user:
            add_user(user_data["id"], user_data["name"])
            user = get_user(user_data["id"])
        return user
    except Exception as e:
        print("DB problem", e)
        raise HTTPException(status_code=500, detail="DB problem")