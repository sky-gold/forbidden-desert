from pydantic import BaseModel

class ChangeNameRequest(BaseModel):
    new_name: str