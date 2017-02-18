from wifi import Cell, Scheme
import os
import time

interface = 'wlan0'
ssid = 'JF2'
passkey = 'JF08310122'
_WIFI_NOT_FOUND = True

os.system('sudo ifdown '+interface)
os.system('sudo ifup '+interface)
print 'waiting for reset...'
time.sleep(10)

cells = Cell.all(interface)
for cell in cells:
    if str(cell.ssid) == ssid:
        print 'SSID found, please wait...'

#        cell = Cell.all('wlan0')[1]
        scheme = Scheme.for_cell(interface, ssid, cell, passkey)
        if bool(Scheme.find(interface, ssid))==False:
            if os.access('/etc/network/interfaces', os.W_OK)!=True:
                os.system('sudo chmod 777 /etc/network/interfaces')
            scheme.save()
            print 'Writing config to interfaces file...'

        scheme.activate()
        _WIFI_NOT_FOUND = False
        break

if _WIFI_NOT_FOUND:
    print 'Designated SSID not found.'
else:
    print 'Wifi is available now!'


