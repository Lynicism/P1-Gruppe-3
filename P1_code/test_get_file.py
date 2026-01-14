import os # Giver adgang til operativsystem-funktioner
import datetime  # Bruges til at arbejde med datoer og tid
import time # Bruges til timestamps
import csv
import matplotlib.pyplot as plt # Bruges til at lave grafer

os.chdir(os.path.dirname(os.path.abspath(__file__)))  # Skifter arbejdsmappe til den mappe, hvor dette Python-script ligger så programmet altid kan finde CSV-filen, uanset hvor det køres fra

values = []
    

date_input1 = input("Enter date (YYYY-MM-DD): ") # Beder brugeren indtaste startdato
date_input2 = input("Enter date (YYYY-MM-DD): ") # Beder brugeren indtaste slutdato

# Konverterer tekst-input til datetime-objekter
dt1=datetime.datetime.strptime(date_input1, "%Y-%m-%d") 
dt2 =datetime.datetime.strptime(date_input2, "%Y-%m-%d")

dt1 = dt1.replace(hour=0, minute=0, second=0, microsecond=0) # Sætter tidspunktet til kl. 00:00:00 for startdatoen
dt2 = dt2.replace(hour=0, minute=0, second=0, microsecond=0) # Sætter tidspunktet til kl. 00:00:00 for slutdatoen


start_ts = time.mktime(dt1.timetuple()) # Konverterer startdato til timestamp
end_ts = time.mktime(dt2.timetuple())+ 86400  # Konverterer slutdato til timestamp og lægger 1 dag til så hele slutdatoen også tælles med


daily_values = {} # Ordbog til at gemme daglige værdier

with open("MyFile.csv", "r") as df: # Åbner CSV-filen til læsning
    reader = csv.DictReader(df, delimiter=';') # Opretter en CSV-læser, der bruger semikolon som feltseparator
    for row in reader: # Går gennem hver række i CSV-filen
        date = int(row["timestamp"]) # Henter tidsstempel fra rækken og konverterer det til heltal
        value = float(row["value"]) # Henter værdi fra rækken og konverterer det til float

        if date >= start_ts and date < end_ts: # Tjekker om tidsstemplet ligger inden for det angivne datointerval
            day = datetime.datetime.fromtimestamp(date).strftime("%Y-%m-%d") # Konverterer tidsstempel til dato i formatet "YYYY-MM-DD
            daily_values.setdefault(day, []).append(value) # Tilføjer værdien til den tilsvarende dag i ordbogen

days=sorted(daily_values.keys()) # Sorterer dagene i stigende rækkefølge


daily_summ = [sum(values) for values in daily_values.values()] # Beregner den samlede værdi for hver dag
avgprday = sum(daily_summ)/len(daily_summ) # Beregner gennemsnittet pr. dag


# Draw graph
plt.figure(figsize=(10, 5)) # Opretter en figur med specificeret størrelse
plt.bar(days, daily_summ) # Opretter et søjlediagram med dagene på x-aksen og de daglige summer på y-aksen

plt.axhline( # Tilføjer en vandret linje for gennemsnittet
    y=avgprday, 
    color='orange',
    linestyle='--',
    linewidth=2,
    label=f"Average: {avgprday:.1f} L/day"
)
plt.legend()  
plt.xticks(rotation=45)
plt.title(f"Usage Value L/day\n{date_input1} to {date_input2}")
plt.xlabel("Date")
plt.ylabel("Water Usage in L")
plt.tight_layout()  # Justerer layout, så intet overlapper
plt.savefig("Daily_use.png") # Gemmer grafen som en PNG-fil
