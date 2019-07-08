KeDei 6.2 SPI TFT Display Test Program

2017 Tong Zhang<ztong@vt.edu>

- Stay away from KeDei Displays!!! they don't give it a sh.t about source code!!!
- Just in case you got the same display!!!!!!
   hopefully.
- FBTFT Kernel Driver is here : [[https://github.com/lzto/FBTFT_KeDei_35_lcd_v62]]

-![RPI KeDei 6.2 Display](/kedei_62_work.jpg)
-![KeDei 6.2 Display Butt](/kedei_butt.jpg)

- based on FreeWing's code
    https://github.com/FREEWING-JP/RaspberryPi_KeDei_35_lcd_v50
- added 6.2 display init logic
- command logic

you need 7 wires to make kedei 6.2 display work!(not including touch panel)

* VCC - 5v
* GND
* MOSI
* MISO
* SCLK
* L\_CS - Chip select for TFT display (of course!)
* T\_CS - Chip select for xpt2046, you need this in order to make display work!
nasty 🤢
manually yank this line(pull low) when finish sending data through SPI



