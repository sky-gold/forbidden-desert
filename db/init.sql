CREATE TABLE users (
    id SERIAL PRIMARY KEY,
    name VARCHAR(255) NOT NULL,
    registration_date DATE DEFAULT CURRENT_DATE NOT NULL
);

CREATE TABLE games (
    id SERIAL PRIMARY KEY,
    status VARCHAR(20) CHECK (status IN ('LOBBY', 'CANCELED', 'IN_PROGRESS', 'FINISHED')),
    players INTEGER[],
    settings JSONB,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE actions (
    action_id SERIAL PRIMARY KEY,
    game_id INTEGER NOT NULL,
    action_number INTEGER NOT NULL,
    user_id INTEGER,
    info JSONB NOT NULL,
    type VARCHAR(255) NOT NULL,
    UNIQUE (game_id, action_number)
);

CREATE TABLE game_states (
    state_id SERIAL PRIMARY KEY,
    action_id INTEGER NOT NULL,
    state_info JSONB NOT NULL,
    FOREIGN KEY (action_id) REFERENCES actions(action_id)
);