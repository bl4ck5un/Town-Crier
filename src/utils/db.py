import sqlite3

DB_FILE = '/opt/tc/daemon/tc.db'

conn = sqlite3.connect(DB_FILE)

c = conn.cursor()

# c.execute("select * from TransactionRecord where block_number = 899735")

c.close()
conn.close()
