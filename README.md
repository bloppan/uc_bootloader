# Bootloader example for STM32F4 devices
A bootloader allows update the code on a device when it is in the field. The purposal of this example is show how works a bootloader which resides in the code space (flash memory) and can reprogram the rest of the memory. This kind of bootloader is appropiate when you have plenty of code space.
The code is very simple, so there are some improvements that you should do if you want to use this bootloader for production purposes:
- Create an abstraction layer between the HAL and drivers will allow you port the code to others hardware arquitectures.
- Add your communication method. In this example, the communication method is the bus CAN, which is frequently used by vehicle electronic control units (ECUs). 
- Add an algorithm that be able to encrypt and decrypt the communication between the target and host.
- Unit tests are always welcome.

The figure below shows how the program memory (flash) is organized in the MCU.

![memory](https://imgur.com/z43853J.jpg)

The bootloader is alocated at first page of memory program (0x08000000). The execution begins at entry point (reset vector). During bootloader initialization, the state of BOOT_FLAG is checked in order to know if it have to jump to application code or wait an update. The possible values of BOOT_FLAG are:
- BOOT_FLAG = 0: The application code is valid and is alocated at 0x0800E000. The bootloader code will perform a jump to application code.
- BOOT_FLAG = 1: There are not application or it is not vaild. The bootloader will wait an update throught CAN bus.

At the end of the update, the bootloader will check if the checksum calculated is equal to the checksum received from host. If it is, the BOOT_FLAG will be set to 0 and the MCU will be rebooted, in order to execute the application code updated.


