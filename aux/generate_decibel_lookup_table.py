import math

# Configuration
FILENAME = "./Ding/core/db_lut_data.cpp"
MIN_DB = -96.0
MAX_DB = 12.0

N_ENTRIES = 2048  # 2 pages
DB_RANGE = MAX_DB - MIN_DB
STEP = DB_RANGE / (N_ENTRIES - 1)


def db_to_gain(db):
    return math.pow(10.0, db / 20.0)


values = dict()

for i in range(N_ENTRIES):
    db = MIN_DB + i * STEP
    values[db] = db_to_gain(db)


with open(FILENAME, "w") as f:
    f.write("#include <array>\n\n")
    f.write(f"static constexpr std::array<float, {N_ENTRIES}> db_lut_data = {{\n")
    for db in sorted(values):
        gain = values[db]
        f.write(f"    {gain:<15.8e}f, // {db:>7.2f} dB\n")
    f.write("}\n")
