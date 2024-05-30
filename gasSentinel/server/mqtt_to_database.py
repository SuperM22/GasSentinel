import sqlite3
import paho.mqtt.client as mqtt
import json

# MQTT settings
MQTT_BROKER = "mqtt.eclipseprojects.io"
MQTT_PORT = 8883
MQTT_TOPIC = "/topic/qos0"

# SQLite database settings
DB_NAME = "iot_data.db"
TABLE_NAME = "gas_data"

# Function to initialize the SQLite database
def init_db():
    try:
        print("Initializing the database...")
        conn = sqlite3.connect(DB_NAME)
        cursor = conn.cursor()
        cursor.execute(f'''
            CREATE TABLE IF NOT EXISTS {TABLE_NAME} (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
                device_id TEXT,
                gas_level_agg TEXT,
                alarm_time TEXT
            )
        ''')
        conn.commit()
        conn.close()
        print("Database initialized.")
    except Exception as e:
        print(f"Error initializing database: {e}")

# Function to store data in the database
def store_in_db(device_id, gas_level_agg, alarm_time):
    try:
        conn = sqlite3.connect(DB_NAME)
        cursor = conn.cursor()
        insert_statement = "INSERT INTO gas_data (device_id, gas_level_agg, alarm_time) VALUES (?, ?, ?)"
        cursor.execute(insert_statement, (device_id, gas_level_agg, alarm_time))
        conn.commit()
        conn.close()
        print("Data stored in database.")
    except Exception as e:
        print(f"Error storing data in database: {e}")



# Function to parse MQTT message and store data in the database
def process_mqtt_message(message):
    try:
        # Parse the JSON string into a Python dictionary
        data = json.loads(message)

        # Extract relevant fields from the dictionary
        device_id = data.get("device_id")
        gas_level_agg = data.get("gas_level_agg")
        alarm_time = data.get("alarm_time")

        # Store the data in the database
        store_in_db(device_id, gas_level_agg, alarm_time)
    except json.JSONDecodeError as e:
        print(f"Error decoding JSON: {e}")
    except Exception as e:
        print(f"Error processing MQTT message: {e}")


# Define MQTT callbacks
def on_connect(client, userdata, flags, rc):
    print(f"Connected with result code {rc}")
    if rc == 0:
        print(f"Subscribing to topic: {MQTT_TOPIC}")
        client.subscribe(MQTT_TOPIC)
    else:
        print(f"Failed to connect, return code {rc}")

def on_message(client, userdata, msg):
        print(f"Received message on topic {msg.topic}")
        payload = msg.payload.decode()
        print(f"Message payload: {payload}")
        process_mqtt_message(payload)


# Initialize the database
init_db()

# Open a persistent connection to the database
db_conn = sqlite3.connect(DB_NAME)

# Initialize MQTT client
client = mqtt.Client(userdata={'db_conn': db_conn})
client.on_connect = on_connect
client.on_message = on_message

# Use TLS for secure connection
print("Setting up TLS...")
client.tls_set()  # default certification authority file

# Connect to the MQTT broker
print(f"Connecting to MQTT broker at {MQTT_BROKER}:{MQTT_PORT} using TLS...")
client.connect(MQTT_BROKER, MQTT_PORT, 60)

# Start the MQTT client loop to process messages
print("Starting MQTT client loop...")
client.loop_forever()

# Close the database connection on program exit
db_conn.close()
print("Database connection closed.")
