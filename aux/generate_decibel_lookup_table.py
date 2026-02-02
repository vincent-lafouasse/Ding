import math

# Configuration
FILENAME = "db_lut.inc"
MIN_DB = -96.0
MAX_DB = 12.0
N_PAGES = 2  # maybe 1 suffices but let's go at it

N_ENTRIES = N_PAGES * 1024  # float[1024] takes 4KB
DB_RANGE = MAX_DB - MIN_DB
STEP = DB_RANGE / (ENTRIES - 1)


def db_to_gain(db):
    return math.pow(10.0, db / 20.0)


# with open(FILENAME, "w") as f:
