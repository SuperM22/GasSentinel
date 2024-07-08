import sqlite3
import paho.mqtt.client as mqtt
import json
import smtplib
import time
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText
import requests
import folium 
from datetime import datetime, timedelta

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
RECIPIENT = "awesomekanha@gmail.com"  # Update with recipient email address
EMAIL_SUBJECT = "Gas Leak Alert"

# Global variable to track the last email sent time
last_email_time = 0
EMAIL_INTERVAL = 900  # 15 minutes in seconds

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
                alarm_time TEXT,
                latitude REAL,
                longitude REAL
            )
        ''')
        conn.commit()
        conn.close()
        print("Database initialized.")
    except Exception as e:
        print(f"Error initializing database: {e}")

# Function to store data in the database
def store_in_db(device_id, gas_level_agg, alarm_time, latitude, longitude ,flag):
    print("AHBFKJABFKJABFKJABFUJAEBFOLAEF")
    try:
        conn = sqlite3.connect(DB_NAME)
        cursor = conn.cursor()
        if flag == "1":
            print("WIFI")
            insert_statement = "INSERT INTO gas_data (device_id, gas_level_agg, alarm_time, latitude, longitude) VALUES (?, ?, ?, ?, ?)"
            cursor.execute(insert_statement, (device_id, gas_level_agg, alarm_time, latitude, longitude))
            conn.commit()
            conn.close()
            print("Data stored in database.")
        elif flag == "0":
            print("NOWIFI")
            # Query to check if data exists within the last 10 minutes for the device_id
            cursor.execute("""
                SELECT 1 FROM gas_data 
                WHERE device_id = ? AND 
                    timestamp >= Datetime('now', '-10 minutes')
            """, (device_id,))
            print("AHFASJFJNAS FHJAVFHAVSFHBF")
            existing_data = cursor.fetchone()

            if existing_data:
                print("Data already exists in database within the last 10 minutes.")
            else:
                # Insert new data into the database
                insert_statement = "INSERT INTO gas_data (device_id, gas_level_agg, alarm_time, latitude, longitude) VALUES (?, ?, ?, ?, ?)"
                cursor.execute(insert_statement, (device_id, gas_level_agg, alarm_time, latitude, longitude))
                conn.commit()  # Commit the transaction
                print("Data stored in database")
    except Exception as e:
        print(f"Error storing data in database: {e}")

# Function to send an email alert
def send_email_alert(device_id, gas_level_agg, alarm_time):
    global last_email_time
    current_time = time.time()
    if current_time - last_email_time < EMAIL_INTERVAL:
        print("Not allowing multiple alerts within 15 minutes.")
        return

    body = f"Gas leak detected!\n\nDevice ID: {device_id}\nGas Level: {gas_level_agg}\nAlarm Time: {alarm_time}"
    msg = MIMEText(body)
    msg['From'] = SENDER
    msg['To'] = RECIPIENT
    msg['Subject'] = EMAIL_SUBJECT

    try:
        with smtplib.SMTP_SSL('smtp.gmail.com', 465) as smtp_server:
            smtp_server.login(SMTP_USERNAME, SMTP_PASSWORD)
            smtp_server.sendmail(SENDER, RECIPIENT, msg.as_string())
            print("Email alert sent.")
            last_email_time = current_time
    except Exception as e:
        print(f"Failed to send email: {e}")

# Function to parse MQTT message and store data in the database
def process_mqtt_message(client, userdata, msg):
    try:
        payload = msg.payload.decode()
        print(f"Received message on topic {msg.topic}")
        print(f"Message payload: {payload}")

         # Replace single quotes with double quotes in the JSON payload
        corrected_payload = payload.replace("'", '"')
        print(f"Corrected payload: {corrected_payload}")

        # Parse the JSON string into a Python dictionary
        data = json.loads(corrected_payload)
        # Extract relevant fields from the dictionary
        device_id = data.get("device_id")
        gas_level_agg = data.get("gas_level_agg")
        alarm_time = data.get("alarm_time")
        bssid = data.get("bssid")  # Assuming 'bssid' is sent in the MQTT message
        flag = data.get("wifi")
        address = data.get("address")
        
        # Get location from Google Geolocation API based on BSSID
        if address == 'MyAddress':
            location = get_location_from_api(bssid)
            if location:
                latitude = location.get("location").get("lat")
                longitude = location.get("location").get("lng")        
            else:
                latitude = None
                longitude = None
        #Get location from Google Geocode API based on the address provided
        else:
            location = get_coordinates_from_address(address)
            print(location)
            if location:
                latitude = location.get("lat")
                longitude = location.get("lng")
            else:
                latitude = None
                longitude = None
            #create map
            create_gas_leak_map(latitude , longitude)
        

        # Store the data in the database
        store_in_db(device_id, gas_level_agg, alarm_time, latitude, longitude, flag)

        # Send an email alert if gas level exceeds the threshold (if needed)
        if float(gas_level_agg) > 2000:  # Adjust threshold as needed
            send_email_alert(device_id, gas_level_agg, alarm_time)
    except json.JSONDecodeError as e:
        print(f"Error decoding JSON: {e}")
    except Exception as e:
        print(f"Error processing MQTT message: {e}")

# Function to fetch location data using Google Geolocation API
def get_location_from_api(bssid):
    try:
        url = f"https://www.googleapis.com/geolocation/v1/geolocate?key=AIzaSyAY9EnVfuNClEa2DvbJQri8MnBojJ2zQ_Q"
        headers = {"Content-Type": "application/json"}
        data = {
            "wifiAccessPoints": [
                {
                    "macAddress": bssid,
                    "signalStrength": -65,
                    "signalToNoiseRatio": 40
                }
            ]
        }
        response = requests.post(url, headers=headers, json=data)
        if response.status_code == 200:
            return response.json()
        else:
            print(f"Error fetching location from API: {response.status_code}")
            return None
    except Exception as e:
        print(f"Exception in API call: {e}")
        return None
#Function to get latitude and longitude based on the address provided
def get_coordinates_from_address(address):
    try:
        url = f"https://maps.googleapis.com/maps/api/geocode/json?address={address}&key=AIzaSyAY9EnVfuNClEa2DvbJQri8MnBojJ2zQ_Q"
        response = requests.get(url)
        if response.status_code == 200:
            result = response.json()
            if result['status'] == 'OK':
                location = result['results'][0]['geometry']['location']
                return location
    except Exception as e:
        print(f"Exception in geocoding API call: {e}")
        return None

map_elements = []
# Function to load map elements from the database
def load_map_elements():
    global map_elements
    try:
        conn = sqlite3.connect(DB_NAME)
        cursor = conn.cursor()
        cursor.execute(f"SELECT latitude, longitude FROM {TABLE_NAME}")
        rows = cursor.fetchall()
        conn.close()
        map_elements = [
            {'latitude': row[0], 'longitude': row[1], 'radius': 100}
            for row in rows if row[0] is not None and row[1] is not None
        ]
        print("Loaded map elements from database.")
    except Exception as e:
        print(f"Error loading map elements from database: {e}")

# Function to count the number of gas leaks in the area around a given latitude and longitude
def count_gas_leaks(latitude, longitude, radius=1000):
    try:
        conn = sqlite3.connect(DB_NAME)
        cursor = conn.cursor()
        query = f"""
        SELECT COUNT(*) FROM {TABLE_NAME}
        WHERE (latitude BETWEEN ? AND ?)
        AND (longitude BETWEEN ? AND ?)
        """
        lat_min = latitude - (radius / 111000.0)  # 1 degree latitude ~ 111 km
        lat_max = latitude + (radius / 111000.0)
        lng_min = longitude - (radius / (111000.0 * abs(latitude)))
        lng_max = longitude + (radius / (111000.0 * abs(latitude)))
        cursor.execute(query, (lat_min, lat_max, lng_min, lng_max))
        count = cursor.fetchone()[0]
        conn.close()
        return count
    except Exception as e:
        print(f"Error counting gas leaks: {e}")
        return 0
    
#Function to create gas-leak map
def create_gas_leak_map(latitude, longitude, radius = 1000):

    # Add the circle and marker to the global list
    map_elements.append({
        'latitude': latitude,
        'longitude': longitude,
        'radius': radius
    })
    
    # Create map with the lat and lng as the center
    m = folium.Map(location=[latitude, longitude], zoom_start=15)

    # Iterate over the global list to add all circles and markers
    for element in map_elements:
        lat = element['latitude']
        lng = element['longitude']
        leak_count = count_gas_leaks(lat, lng, element['radius'])
        popup_text = f"Gas Leak Detected\nLeaks in Area: {leak_count}"
        folium.Circle(
            location=[element['latitude'], element['longitude']],
            radius=element['radius'],
            color='red',
            fill=True,
            fill_color='red',
            opacity = 0.5,
            fill_opacity=0.2
        ).add_to(m)
        
        folium.Marker(
            location=[element['latitude'], element['longitude']],
            popup= popup_text,
            icon=folium.Icon(color='red', icon='info-sign')
        ).add_to(m)

    map_file = 'gas_leak_map.html'
    m.save(map_file)
    print(f"Map saved to {map_file}")

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
# Load map elements
load_map_elements()

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
