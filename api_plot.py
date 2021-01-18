import requests
import numpy as np
import matplotlib.pyplot as plt

entry_id = np.empty(0)
weight = np.empty(0)
ir = np.empty(0)
date = np.empty(0)
ver = np.empty(0)

url = 'https://api.thingspeak.com/channels/1268523/feeds.json?api_key=DOU3H2K9AHMNCBR9'

bin_req = requests.get(url)
bin_dict = bin_req.json()
for feeds in bin_dict['feeds']:
    entry_id = np.append(entry_id, feeds['entry_id'])
    weight = np.append(weight, float(feeds['field1']))
    ir = np.append(ir, float(feeds['field2']))
    date = np.append(date, feeds['field3'].split(" ")[1]+" "+feeds['field3'].split(" ")[2])
    ver = np.append(ver, feeds['field4'])

fig, (ax1, ax2, ax3, ax4) = plt.subplots(4, sharex=True)
ax1.plot(entry_id, weight, 'tab:red')
ax2.plot(entry_id, ir, 'tab:green')
ax3.plot(entry_id, date, 'tab:blue')
ax4.plot(entry_id, ver, 'tab:orange')

fig.text(0.5, 0.92, 'Smart Bin Data Monitoring', ha='center',fontsize=12)
fig.text(0.5, 0.02, 'Entry_id', ha='center')
fig.text(0.02, 0.8, 'Weight', va='center', rotation='vertical')
fig.text(0.05, 0.8, '(in kg)', va='center', rotation='vertical')
fig.text(0.02, 0.6, '# of IRs', va='center', rotation='vertical')
fig.text(0.02, 0.4, 'Date', va='center', rotation='vertical')
fig.text(0.02, 0.2, 'Version', va='center', rotation='vertical')
plt.subplots_adjust(left=0.17, bottom=None, right=None, top=None, wspace=None, hspace=None)

for a in [ax1, ax2, ax3, ax4]:
        a.grid(zorder=0, linestyle='--')
        a.grid(True)

plt.show()


