EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
Text GLabel 6400 4500 2    50   Input ~ 0
galvoY
Wire Wire Line
	5300 2750 1850 2750
Wire Wire Line
	1850 2750 1850 3100
Wire Wire Line
	2900 4900 2750 4900
$Comp
L Connector:Raspberry_Pi_2_3 J?
U 1 1 61B80C86
P 1950 4400
F 0 "J?" H 1950 5881 50  0000 C CNN
F 1 "Raspberry_Pi_2_3" H 1950 5790 50  0000 C CNN
F 2 "" H 1950 4400 50  0001 C CNN
F 3 "https://www.raspberrypi.org/documentation/hardware/raspberrypi/schematics/rpi_SCH_3bplus_1p0_reduced.pdf" H 1950 4400 50  0001 C CNN
	1    1950 4400
	1    0    0    -1  
$EndComp
Wire Wire Line
	3000 4800 2750 4800
Wire Wire Line
	4800 4500 2750 4500
Wire Wire Line
	2900 4900 2900 4200
Wire Wire Line
	2900 4200 4800 4200
Wire Wire Line
	3000 4800 3000 4400
Wire Wire Line
	3000 4400 4800 4400
Wire Wire Line
	4800 4300 5200 4300
Wire Wire Line
	5200 4300 5200 6000
Wire Wire Line
	5300 6100 5300 4800
$Comp
L Diode_Laser:PL450B LD?
U 1 1 61BAD014
P 7150 4250
F 0 "LD?" H 7100 4535 50  0000 C CNN
F 1 "PL450B" H 7100 4444 50  0000 C CNN
F 2 "OptoDevice:LaserDiode_TO38ICut-3" H 7150 4075 50  0001 C CNN
F 3 "http://www.osram-os.com/Graphics/XPic5/00193831_0.pdf/PL%20450B.pdf" H 7180 4050 50  0001 C CNN
	1    7150 4250
	0    -1   -1   0   
$EndComp
Wire Wire Line
	5300 2750 5300 3900
$Comp
L Analog_DAC:MCP4822 U?
U 1 1 61B7B705
P 5300 4300
F 0 "U?" H 5300 4881 50  0000 C CNN
F 1 "MCP4822" H 5300 4790 50  0000 C CNN
F 2 "" H 6100 4000 50  0001 C CNN
F 3 "http://ww1.microchip.com/downloads/en/DeviceDoc/20002249B.pdf" H 6100 4000 50  0001 C CNN
	1    5300 4300
	1    0    0    -1  
$EndComp
Text GLabel 5800 4200 2    50   BiDi ~ 0
5V->15V(1:3)
Text GLabel 6400 4200 2    50   Input ~ 0
galvoX
Text GLabel 5800 4500 2    50   BiDi ~ 0
5V->15V(1:3)
$Comp
L Isolator:HCNW2201 U?
U 1 1 61BC14B7
P 6850 3500
F 0 "U?" H 7194 3546 50  0000 L CNN
F 1 "HCNW2201" H 7194 3455 50  0000 L CNN
F 2 "Package_DIP:DIP-8_W10.16mm" H 6850 3500 50  0001 C CNN
F 3 "https://docs.broadcom.com/docs/AV02-0674EN" H 6850 3500 50  0001 C CNN
	1    6850 3500
	1    0    0    -1  
$EndComp
Wire Wire Line
	1000 2600 1000 3900
Wire Wire Line
	1000 3900 1150 3900
Wire Wire Line
	1650 6200 1650 5700
Wire Wire Line
	5200 6000 1850 6000
Wire Wire Line
	5300 6100 1750 6100
Wire Wire Line
	1750 6100 1750 5700
Wire Wire Line
	1850 5650 1850 6000
Wire Wire Line
	7150 4050 7150 3500
Wire Wire Line
	1000 2600 6550 2600
Wire Wire Line
	6550 2600 6550 3400
Wire Wire Line
	1650 6200 6550 6200
Wire Wire Line
	6550 6200 6550 3600
$Comp
L power:+15V #PWR?
U 1 1 61BCBA54
P 6850 3200
F 0 "#PWR?" H 6850 3050 50  0001 C CNN
F 1 "+15V" H 6865 3373 50  0000 C CNN
F 2 "" H 6850 3200 50  0001 C CNN
F 3 "" H 6850 3200 50  0001 C CNN
	1    6850 3200
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 61BCC060
P 7150 4550
F 0 "#PWR?" H 7150 4300 50  0001 C CNN
F 1 "GND" H 7155 4377 50  0000 C CNN
F 2 "" H 7150 4550 50  0001 C CNN
F 3 "" H 7150 4550 50  0001 C CNN
	1    7150 4550
	1    0    0    -1  
$EndComp
$EndSCHEMATC
