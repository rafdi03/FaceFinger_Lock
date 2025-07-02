import sqlite3

conn = sqlite3.connect('absensi.db')
c = conn.cursor()

# Tabel user: id, nama, encoding (disimpan sebagai BLOB)
c.execute('''
CREATE TABLE IF NOT EXISTS users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    nama TEXT NOT NULL,
    encoding BLOB NOT NULL
)
''')

# Tabel absensi: id, user_id, waktu
c.execute('''
CREATE TABLE IF NOT EXISTS absensi (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER,
    waktu DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY(user_id) REFERENCES users(id)
)
''')

# Tabel unknown_faces: id, filename, waktu
c.execute('''
CREATE TABLE IF NOT EXISTS unknown_faces (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    filename TEXT NOT NULL,
    waktu DATETIME DEFAULT CURRENT_TIMESTAMP
)
''')

conn.commit()
conn.close()