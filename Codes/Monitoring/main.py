import requests
from string import Template
import json
import matplotlib.pyplot as plt
from datetime import datetime, timedelta
from matplotlib.dates import DateFormatter

API_KEYS = {
    "SmartBin_Data":{
        "CH_ID" : "1268523",
        "WRITE": "H1VXBE4MWXH7D302",
        "READ": "DOU3H2K9AHMNCBR9"
    },
    "SmartBin_Prediction":{
        "CH_ID" : "1334540",
        "WRITE": "I0E4WPLETXZHBXWI",
        "READ": "UVNUJDZI2TMUKBYW"
    }
}

END_POINTS = {
    "READ" : {
        "CHANNEL_FEED" : Template("https://api.thingspeak.com/channels/${CH_ID}/feeds.json?api_key=${READ}")
    }
}

SmartBins = {
    "SB_NITT_1" : {
        "Address": "Admin Block, NITT",
        "Mobile": "9470186194"
    },
    "SB_NITT_2" : {
        "Address": "Dept of EEE, NITT",
        "Mobile": "9470186194"
    },
    "SB_NITT_3" : {
        "Address": "Orion, NITT",
        "Mobile": "9470186194"
    }
}

def getChannelFeed(url):
    res = requests.get(url)
    if res.status_code==200:
        return json.loads(res.text)
    else:
        return None

def getData():
    data = getChannelFeed(END_POINTS["READ"]["CHANNEL_FEED"].substitute(CH_ID=API_KEYS["SmartBin_Data"]["CH_ID"],READ=API_KEYS["SmartBin_Data"]["READ"]))
    prediction = getChannelFeed(END_POINTS["READ"]["CHANNEL_FEED"].substitute(CH_ID=API_KEYS["SmartBin_Prediction"]["CH_ID"],READ=API_KEYS["SmartBin_Prediction"]["READ"]))
    return data, prediction

def process():
    data, prediction = getData()
    # Digest data feed
    finalData = {}
    data_feed = data['feeds']
    for row in data_feed:
        if row['field1'] in finalData.keys():
            finalData[row['field1']].append({
                "date": row['created_at'],
                "firmwareVersion": row['field2'],
                "currentWeight": row['field3'],
                "currentHeight": row['field4'],
                "weightThreshold": row['field5'],
                "heightThreshold": row['field6'],
            })
        else:
            finalData[row['field1']] = [{
                "date": row['created_at'],
                "firmwareVersion": row['field2'],
                "currentWeight": row['field3'],
                "currentHeight": row['field4'],
                "weightThreshold": row['field5'],
                "heightThreshold": row['field6'],
            }]
    # Digest prediction feed 
    finalPrediction = {}
    prediction_feed = prediction['feeds']
    for row in prediction_feed:
        if row['field1'] in finalPrediction.keys():
            finalPrediction[row['field1']].append({
                "date": row['created_at'],
                "predictedTime": row['field2'],
            })
        else:
            finalPrediction[row['field1']] = [{
                "date": row['created_at'],
                "predictedTime": row['field2'],
            }]
    return finalData, finalPrediction

def matlab(data, DB_id, prediction):
    weight = []
    height = []
    dates = []
    for dict in data:
        weight.append(dict['currentWeight'])
        tempDate = dict['date'].replace("T", " ").replace("Z","")
        time_change = timedelta(0,19800)
        actualTime = datetime.strptime(tempDate,"%Y-%m-%d %H:%M:%S")
        finalTime = actualTime + time_change
        dates.append(finalTime)
        height.append(int(dict['currentHeight'], 2))
    lastDate = dates[len(dates)-1]
    firmWare = data[len(data)-1]["firmwareVersion"]
    # Getting predicted date
    finalFillDate = -1
    if prediction is None:
        finalFillDate = "Nan"
    else:
        finalFillDate = prediction[len(prediction)-1]["predictedTime"]

    # Plotting the data using Matplotlib
    fig, (ax1, ax2) = plt.subplots(2, sharex=True)
    ax1.plot(dates, weight, 'tab:red', marker='o')
    ax1.set_ylabel('Weight (KG)',weight="bold") 
    ax2.plot(dates, height, 'tab:blue', marker='o')
    ax2.set_ylabel('# of IRs',weight="bold") 

    fig.text(0.5, 0.96, 'Smart Bin Data Monitoring', ha='center',fontsize=12,
                bbox ={'facecolor':'green','alpha':0.6,'pad':5})
    fig.text(0.5, 0.90, DB_id, ha ='center',weight="bold")
    fig.text(0.95, 0.90, "Expected Fill By "+str(finalFillDate)+" days from now", ha ='right',weight="bold")
    fig.text(0.01, 0.035, "Last Update: "+str(lastDate), va='center',weight="bold")
    fig.text(0.99, 0.035, "Firmware Version: "+firmWare, va='center',ha="right",weight="bold")
    fig.text(0.5, 0, '(c) 2021 RAR', verticalalignment ='bottom', horizontalalignment ='center', color ='green', weight="bold")
    plt.subplots_adjust(left=None, bottom=0.2, right=None, top=None, wspace=None, hspace=None)

    date_form = DateFormatter("%d-%m-%y, %H:%M:%S")
    for a in [ax1, ax2]:
        a.grid(zorder=0, linestyle='--')
        a.grid(True)
        a.xaxis.set_major_formatter(date_form)
    plt.xticks(rotation=30)

    plt.show()

    return str(lastDate), firmWare, str(finalFillDate)


def menu():
    ch = 'y'
    while ch=='y':
        print("")
        print("Getting fresh data from the server........")
        data, prediction = process()
        print("DONE\n")
        print("Data of "+str(len(data.keys()))+" Smart Bins are present in the record.\n")
        print("Various SmartBins with their address are: \n")
        i = 1
        for key in data.keys():
            addr = ""
            if key in SmartBins.keys():
                addr = SmartBins[key]["Address"]
            else:
                addr = "NaN"
            print(str(i)+". "+key +" <-> "+ addr)
            i=i+1
        print("\nWhich one do you choose:\nAns:")
        choice = int(input())
        choiceKey = list(data.keys())[choice-1]
        predictData = None
        if choiceKey in prediction.keys():
            predictData = prediction[choiceKey]
        print("")
        print("Analyzing the SmartBin for its record.....\n")
        lastDate, firmware, expected  = matlab(data[choiceKey], choiceKey, predictData)
        print("Latest Firmware Version: "+firmware)
        print("Last Updated On: "+lastDate)
        print("Expected Fill By "+expected+" days from now")
        print("")
        ch = input("Again? (Yes='y', No='n'): ")
    
    exitProgram()

def exitProgram():
    print("")
    print("Thank you for using Smart Bin Monitoring")
    print("(c) RAR, EEE 4th Year 2021 Batch, NITT")
    print("Bye.")
    exit(0)

if __name__ == "__main__":
    menu()
