######## first install "nginx" server
sudo apt-get install nginx
# to check service, we can type:
service nginx status
# test by web browser (you must make sure that "index.html" in /var/www/html is exist for nginx server)

######## for RPi_Cam_Web_Interface, we need to install "php" 
sudo apt-get install php5-fpm

#設定 NGINX 伺服器，開啟 /etc/nginx/sites-available/default 這個設定檔，找到這一段：
# Add index.php to the list if you are using PHP
index index.html index.htm index.nginx-debian.html;

#加入 index.php，修改成這樣：
# Add index.php to the list if you are using PHP
index index.php index.html index.htm index.nginx-debian.html;

#同樣在 /etc/nginx/sites-available/default 這個設定檔，找到這一段：
# pass the PHP scripts to FastCGI server listening on 127.0.0.1:9000
#
#location ~ \.php$ {
#       include snippets/fastcgi-php.conf;
#
#       # With php5-cgi alone:
#       fastcgi_pass 127.0.0.1:9000;
#       # With php5-fpm:
#       fastcgi_pass unix:/var/run/php5-fpm.sock;
#}

#把這一段的註解拿掉，而 fastcgi_pass 的部分則選擇 php5-fpm 的設定，php5-cgi 的版本要註解起來，變成這樣：
# pass the PHP scripts to FastCGI server listening on 127.0.0.1:9000

location ~ \.php$ {
       include snippets/fastcgi-php.conf;

       # With php5-cgi alone:
       # fastcgi_pass 127.0.0.1:9000;
       # With php5-fpm:
       fastcgi_pass unix:/var/run/php5-fpm.sock;
}

#讓 NGINX 重新設定檔：
sudo service nginx reload

####### now we install RPi_Cam_Web_Interface
#Step 1: Install Raspbian on your RPi
#Step 2: Attach camera to RPi and enable camera support (http://www.raspberrypi.org/camera)
#Step 3: Update your RPi with the following commands:

sudo apt-get update
sudo apt-get dist-upgrade
cd
cd Downloads
git clone https://github.com/silvanmelchior/RPi_Cam_Web_Interface.git
cd RPi_Cam_Web_Interface
./install.sh

#Autostart should be "yes" if you want this software to start automatically when raspberry boots up. 
#jpglink should normally be "no". Change it to yes if you have external software that needs direct access to the cam.jpg image file.

#Open up any browser on any computer in your network and enter the url to access the camera web site. 
#This will be http://ipAddress:port/subfolder. 
#If the port had been left at default 80 during install then this may be left out. 
#Similarly the subfolder (default html) can be left out if that was cleared during the install. 
#So for a port 80, no subfolder install the url becomes http://ipAddress

