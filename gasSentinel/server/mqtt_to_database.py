import sqlite3
import paho.mqtt.client as mqtt
import json
import smtplib
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText

# MQTT settings
MQTT_BROKER = "mqtt.eclipseprojects.io"
MQTT_PORT = 1883
MQTT_TOPIC = "/topic/qos0"

# SQLite database settings
DB_NAME = "iot_data.db"
TABLE_NAME = "gas_data"

# Email settings
SMTP_USERNAME = "gassentinel@gmail.com"  # Update with your email address
SMTP_PASSWORD = "xlpm speg pgfu lkwo"  # Update with your email password
SENDER = "gassentinel@gmail.com"  # Update with your email address
RECIPIENT = "valiqureshi2000@gmail.com"  # Update with recipient email address
EMAIL_SUBJECT = "Gas Leak Alert"

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

# Function to send an email alert
def send_email_alert(device_id, gas_level_agg, alarm_time):
        body = f"Gas leak detected!\n\nDevice ID: {device_id}\nGas Level: {gas_level_agg}\nAlarm Time: {alarm_time}"
        msg = MIMEText(body)
        msg['From'] = SENDER
        msg['To'] = RECIPIENT
        msg['Subject'] = EMAIL_SUBJECT

        with smtplib.SMTP_SSL('smtp.gmail.com', 465) as smtp_server:
            smtp_server.login(SMTP_USERNAME, SMTP_PASSWORD)
            smtp_server.sendmail(SENDER, RECIPIENT, msg.as_string())
            print("Email alert sent.")

# Function to parse MQTT message and store data in the database
def process_mqtt_message(client, userdata, msg):
    try:
        payload = msg.payload.decode()
        print(f"Received message on topic {msg.topic}")
        print(f"Message payload: {payload}")

        # Parse the JSON string into a Python dictionary
        data = json.loads(payload)

        # Extract relevant fields from the dictionary
        device_id = data.get("device_id")
        gas_level_agg = data.get("gas_level_agg")
        alarm_time = data.get("alarm_time")

        # Store the data in the database
        store_in_db(device_id, gas_level_agg, alarm_time)

        # Send an email alert if gas level exceeds the threshold
        if float(gas_level_agg) > 2000:  # Adjust threshold as needed
            send_email_alert(device_id, gas_level_agg, alarm_time)
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
    process_mqtt_message(client, userdata, msg)

# Initialize the database
init_db()

# Open a persistent connection to the database
db_conn = sqlite3.connect(DB_NAME)

# Initialize MQTT client
client = mqtt.Client(userdata={'db_conn': db_conn})
client.on_connect = on_connect
client.on_message = on_message

# Connect to MQTT broker
try:
    print(f"Connecting to MQTT broker at {MQTT_BROKER}:{MQTT_PORT}...")
    client.connect(MQTT_BROKER, MQTT_PORT, 60)
    print(f"Connected to MQTT broker at {MQTT_BROKER}:{MQTT_PORT}")
except Exception as e:
    print(f"Failed to connect to MQTT broker: {e}")
    exit(1)

# Start the MQTT client loop to process messages
client.loop_forever()

# Close the database connection on program exit
db_conn.close()
print("Database connection closed.")
