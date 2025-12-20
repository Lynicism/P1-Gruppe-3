import os
import datetime
import time
import csv
import matplotlib.pyplot as plt

os.chdir(os.path.dirname(os.path.abspath(__file__)))

values = []
    

date_input1 = input("Enter date (YYYY-MM-DD): ")
date_input2 = input("Enter date (YYYY-MM-DD): ")

dt1=datetime.datetime.strptime(date_input1, "%Y-%m-%d")
dt2 =datetime.datetime.strptime(date_input2, "%Y-%m-%d")

dt1 = dt1.replace(hour=0, minute=0, second=0, microsecond=0) # Returns a copy
dt2 = dt2.replace(hour=0, minute=0, second=0, microsecond=0) # Returns a copy


start_ts = time.mktime(dt1.timetuple())
end_ts = time.mktime(dt2.timetuple())+ 86400  # + one day in seconds


daily_values = {}

with open("MyFile.csv", "r") as df:
    reader = csv.DictReader(df, delimiter=';')
    for row in reader:
        date = int(row["timestamp"])
        value = float(row["value"])

        if date >= start_ts and date < end_ts:
            day = datetime.datetime.fromtimestamp(date).strftime("%Y-%m-%d")
            daily_values.setdefault(day, []).append(value)

days=sorted(daily_values.keys())


daily_summ = [sum(values) for values in daily_values.values()]
avgprday = sum(daily_summ)/len(daily_summ)


# Draw graph
plt.figure(figsize=(10, 5))
plt.bar(days, daily_summ)

plt.axhline(
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
plt.tight_layout()
plt.savefig("Daily_use.png")

#add gennemsnit for perioden