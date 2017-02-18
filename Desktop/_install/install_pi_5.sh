#------------------------
# step.5: modify setting 
#------------------------
# <fixed IP setting>
cd /etc
sudo nano dhcpcd.conf
#interface wlan0 # or etho for wired connection
#static ip_address=192.168.1.110
#static routers=192.168.1.1
#static domain_name_servers=192.168.1.1

# <setup sound car>
alsamixer
# (choose your usb sound card, and setup amp & mic volume)
# for checking, enter:
aplay -l
# edit .asoundrc file
sudo nano ~/.asoundrc
# add: pcm.!default sysdefault:Device
# save
# then use aplay <filename>