# iot-out.lan

## ESP-01 with DB18B20  
outdoor temperature sensor

### awake state  
wake up and measure temp, then send to server  
connect to network, get address from dhcp, get temperature and sent request takes ~3 seconds with power consumption of aprox 70mA  

### deep sleep  
after then ESP goes to deep sleep for 10 minutes  
measured power consumption in deep sleep equals 110uA  

### power source
1500mAh 3.7V lipo battery attached to constant-current/constant-voltage linear charger with battery protection (TP4056)  

#### notice  
ESP-01 is directly connected to battery (voltage range from 3.6 to 4.2v) which is far higher than original 3.3v, and safe 3.5v, but it works like a charm... at least for now ;)
