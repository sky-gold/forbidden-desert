import os
from sqlalchemy import create_engine, text
from sqlalchemy.orm import sessionmaker, Session
from sqlalchemy.exc import IntegrityError

DB_HOST = os.getenv("DB_HOST")
DB_NAME = os.getenv("DB_NAME")
DB_USER = os.getenv("DB_USER")
DB_PASSWORD = os.getenv("DB_PASSWORD")

DATABASE_URL = f"postgresql://{DB_USER}:{DB_PASSWORD}@{DB_HOST}/{DB_NAME}"

engine = create_engine(DATABASE_URL)
Session = sessionmaker(bind=engine)

def get_user(user_id: int):
    print("get_user", user_id)
    with Session() as session:
        print("!", session, type(session))
        user = session.execute(text("SELECT id, name, registration_date FROM users WHERE id = :user_id"), {"user_id": user_id}).fetchone()
        print("user=", user)
        if user:
            return {"id": user[0], "name": user[1], "registration_date": user[2]}
        return None

def add_user(user_id: int, name: str):
    print("add_user")
    print("add_user", user_id, name)
    with Session() as session:
        try:
            session.execute(text("INSERT INTO users (id, name) VALUES (:user_id, :name)"), {"user_id": user_id, "name": name})
            session.commit()
        except IntegrityError:
            session.rollback()
