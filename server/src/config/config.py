"""
Reads your .env file and exposes settings to the rest of the app.
Every value has a safe default so the app starts even without a .env file.

@lru_cache means the .env file is read only once at startup,
not on every single request.
"""

from functools import lru_cache
from pydantic_settings import BaseSettings, SettingsConfigDict


class Settings(BaseSettings):
    db_path:   str  = "./data/markers.db"  # path to the SQLite file
    api_key:   str  = "CHANGE_ME_ANTENNA"         # secret for the antenna hardware
    admin_key: str  = "CHANGE_ME_ADMIN"           # secret for you (ops / management)
    app_name:  str  = "River Monitor API"
    debug:     bool = True                        # never True in production

    model_config = SettingsConfigDict(env_file=".env", env_file_encoding="utf-8")


@lru_cache
def get_settings() -> Settings:
    return Settings()