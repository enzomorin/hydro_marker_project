import os
import csv
import random
from datetime import datetime, timedelta

# ---------------------------
# Créer le dossier de sortie
# ---------------------------
script_dir = os.path.dirname(os.path.abspath(__file__))
output_dir = os.path.join(script_dir, "out")
os.makedirs(output_dir, exist_ok=True)

# ---------------------------
# Nom du fichier CSV unique
# ---------------------------
def unique_csv_filename(name: str) -> str:
    base_name = name.replace(".csv", "")
    counter = 1
    filename = f"{base_name}.csv"
    full_path = os.path.join(output_dir, filename)
    while os.path.exists(full_path):
        filename = f"{base_name}_{counter}.csv"
        full_path = os.path.join(output_dir, filename)
        counter += 1
    return full_path

filename = unique_csv_filename("sample_data.csv")

# ---------------------------
# Nombre de secondes à simuler
# ---------------------------
num_seconds = 600  # par exemple 10 minutes pour test rapide

# ---------------------------
# Fonction pour générer des données aléatoires
# ---------------------------
def generate_sample_data():
    temperature = round(random.uniform(-10, 35), 2)       # °C
    pressure = round(random.uniform(950, 1050), 2)        # hPa
    depth = round(random.uniform(0, 100), 2)              # mètres
    altitude = round(random.uniform(0, 2000), 2)          # mètres
    rpm = round(random.uniform(0, 3000), 2)               # tours/min
    cross_section = round(random.uniform(5, 50), 2)       # m²
    return temperature, pressure, depth, altitude, rpm, cross_section

# ---------------------------
# Générer l'heure de départ
# ---------------------------
start_time = datetime.now()

# ---------------------------
# Écriture du fichier CSV
# ---------------------------
with open(filename, mode="w", newline="") as csv_file:
    writer = csv.writer(csv_file)
    # Écrire l'en-tête
    writer.writerow(["timestamp", "temperature_C", "pressure_hPa", "depth_m",
                    "altitude_m", "mission_time_s", "rpm", "cross_section_m2"])
    
    for second in range(num_seconds):
        timestamp = start_time + timedelta(seconds=second)
        temperature, pressure, depth, altitude, rpm, cross_section = generate_sample_data()
        writer.writerow([timestamp.strftime("%Y-%m-%d %H:%M:%S"),
                        temperature, pressure, depth, altitude,
                        second, rpm, cross_section])
print(f"Fichier CSV généré : {filename}")

# ---------------------------
# Lecture et affichage du CSV
# ---------------------------
print("Contenu du fichier CSV (5 premières lignes) :")
with open(filename, mode="r") as csv_file:
    reader = csv.reader(csv_file)
    for i, row in enumerate(reader):
        print(row)
        if i >= 4: break