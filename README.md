# ARTe demo application
Arduino UNO code for the vocally controlled car.

<img src="imgs/preview1.jpg" alt=" Preview image not available. "/>

## Installation
  1. Download the repository
  2. open the .ino file with Arduino IDE
  3. install the ARTe extension (Arduino Real-Time extension) in your Arduino IDE as described at http://arte.retis.santannapisa.it/
  4. install an Arduino UNO board on a Shield Bot v1.2, as available at http://wiki.seeedstudio.com/Shield_Bot_V1.2/
  5. connect a bluetooth module as described at https://github.com/HerrAugust/skill-smart-car
  6. upload the code to the Arduino UNO board
  7. optionally, install Mycroft and the Mycroft skill as below

## Required: Mycroft skill
Even if this demo works on its own via serial commands (i.e., Arduino IDE serial monitor and keyboard), to fully appreciate the demo with vocal commands you need to also install the Mycroft skill, available at https://github.com/HerrAugust/skill-smart-car. More information in the repository.

## Tests
Arduino UNO code tested on Funduino UNO with Arduino IDE 1.9.0 beta for Linux 64 bit (Ubuntu 18.04 used).
Vocal commands have been given via Mycroft. The bluetooth module HC-06 has been used.

Memo: The Mycroft skill must stay on its own repository to be cloned on Raspberry or PC!
