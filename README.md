# Bootloader example for STM32F1 devices
A bootloader allows update the code on a device when it is in the field. The purposal of this example is show how works a bootloader which resides in the code space (flash memory) and can reprogram the rest of the memory. This kind of bootloader is appropiate when you have plenty of code space.
The code is very simple, so there are some improvements that you should do if you want to use this bootloader for production purposes:
- Create an abstraction layer between the HAL and drivers will allow you port the code to others hardware arquitectures.
- Add your communication method. In this example, the communication method is the bus CAN, which is frequently used by vehicle electronic control units (ECUs). 
- Add an algorithm that be able to encrypt and decrypt the communication between the target and host.
- Unit tests are always welcome.

### FLASH Memory Map
The figure below shows how the program memory flash is organized in the MCU.
![memory](https://imgur.com/TNSTOie.jpg)

The bootloader is alocated at first page of memory program (0x08000000). The execution begins at entry point (reset vector). During bootloader initialization, the state of BOOT_FLAG is checked in order to know if it have to jump to application code or wait an update. The possible values of BOOT_FLAG are:
- BOOT_FLAG = 0: The application code is valid and is alocated at 0x0800E000. The bootloader code will perform a jump to application code.
- BOOT_FLAG = 1: There are not application or it is not vaild. The bootloader will wait an update throught CAN bus.

At the end of the update, the bootloader will check if the checksum calculated is equal to the checksum received from host. If it is, the BOOT_FLAG will be set to 0 and the MCU will be rebooted, in order to execute the application code.

### Application 
The application is the program sent as update. The entry point of application must be configured at address 0x0800E000 on linker script as is shown below. 

![entry_point](https://imgur.com/Lcd5X8k.jpg)

### Updater
The updater has been developed as python script, which import the PCANBasic library in order to send the application binary through CAN bus. 
To send the update, you must specificate the binary file and the id CAN of your target. For example:
```sh
python updater.py firmware.bin 0x321
```
First, the updater will send the size of the update (in bytes) to the target. If the program memory of target is enough to store de new firmware application, the updater will begin to send the whole binary file. At the end, the checksum of binary file will be sent to target.

The wire used to send de update is shown in below figure.
![pcan](https://imgur.com/H4EqOBp.jpg)


# References
- [STM32F103 Datasheet](https://www.st.com/resource/en/datasheet/stm32f103c8.pdf)
- [Nucleo F103 Documentation](https://www.st.com/en/evaluation-tools/nucleo-f103rb.html)
- [PCAN-USB Website](https://www.peak-system.com/)











