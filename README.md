# RaspberryPi_KeDei_35_lcd_v50
Raspberry Pi KeDei 3.5 inch TFT LCD module V5.0 Control program


## Raspberry Pi用 KeDei 3.5インチ TFT液晶を自前のプログラムで制御する方法
http://www.neko.ne.jp/~freewing/raspberry_pi/raspberry_pi_3_tft_lcd_3_5inch_kedei_touch_xpt2046_2/


## References（参考文献）
### KeDei 3.5 inch 480x320 TFT lcd from ali
https://www.raspberrypi.org/forums/viewtopic.php?p=1019562  
 by Conjur - Mon Aug 22, 2016 2:12 am - Final post on the KeDei v5.0 code.

### l0nley/kedei35
https://github.com/l0nley/kedei35


## Build

#### # Enable SPI
sudo raspi-config

#### # build bcm2835 lib.
cd  
wget http://www.airspayce.com/mikem/bcm2835/bcm2835-1.52.tar.gz  
tar -zxf bcm2835-1.52.tar.gz  
cd bcm2835-1.52  
./configure  
sudo make install

#### # git clone
cd  
git clone https://github.com/FREEWING-JP/RaspberryPi_KeDei_35_lcd_v50.git

#### # compile
cd ~/RaspberryPi_KeDei_35_lcd_v50  
gcc -o kedei_lcd_v50_pi kedei_lcd_v50_pi.c -lbcm2835  

#### # execute !
sudo ./kedei_lcd_v50_pi


## Picture

![Raspberry Pi KeDei 3.5 inch LCD module V5.0 Control program](/kedei_35_lcd_v50_module_spi_progrmable.jpg)

![Raspberry Pi KeDei 3.5 inch LCD module V5.0 module](/kedei_35_lcd_v50_module.jpg)

