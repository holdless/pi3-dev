#=======================
#C++ call java:
#=======================
cd
cd /home/pi/Desktop/tango/talk/pos-tagging
g++ -I/usr/lib/jvm/jdk-8-oracle-arm-vfp-hflt/include -I/usr/lib/jvm/jdk-8-oracle-arm-vfp-hflt/include/linux -L/usr/lib/jvm/jdk-8-oracle-arm-vfp-hflt/jre/lib/arm/client -ljvm c_call_java.cpp -o c_call_java

