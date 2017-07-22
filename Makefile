all:
	gcc -o kedei_lcd_v62_pi_wiringpi kedei_lcd_v62_pi_wiringpi.c -lwiringPi
clean:
	rm -f kedei_lcd_v62_pi_wiringpi
