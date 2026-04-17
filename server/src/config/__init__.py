# Makes config/ a Python package.
# Re-exports the two most-used items so the rest of the app can write:
#   from src.config import get_settings, require_admin_key, require_antenna_key
# instead of the full path every time.

from src.config.config import get_settings
from src.config.auth   import require_admin_key, require_antenna_key