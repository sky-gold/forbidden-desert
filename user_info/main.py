from fastapi import FastAPI, Depends, HTTPException, Header


from auth import check_auth
from exceptions import InvalidToken, ExpiredToken
from database import get_user, add_user, change_user_name
from schemas import ChangeNameRequest

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

@app.post("/change/name")
async def change_name(request: ChangeNameRequest, token: str = Depends(get_authorization_header)):
    try:
        user_data = check_auth(token)
    except (InvalidToken, ExpiredToken) as e:
        raise HTTPException(status_code=401, detail=str(e))
    user = get_user(user_data["id"])
    if not user:
        raise HTTPException(status_code=404, detail="User not found")
    try:    
        change_user_name(user_data["id"], request.new_name)
        return {"message": "Name changed successfully"}
    except Exception as e:
        print("DB problem", e)
        raise HTTPException(status_code=500, detail="DB problem")