# Control box

This project is developed for the WeAct Black Pill V2.0 board which use a STM32F411 MCU.

The control box allows to control up to 4 actuators while connected to FlyPT Mover (https://www.flyptmover.com/)

It is configured to work with SRT150 actuators which have 20cm of range (which corresponds to 20 turn of the motor). The drivers have to be configured to 3200 micro steps by turn which result in a total of 64000 micro steps for the full range (this value have to be used to configure FlyPT Mover profile).
