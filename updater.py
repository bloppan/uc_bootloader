
import sys
import time
import os

from PCANBasic import *

#################################### PARAMETROS ##########################################
channel                 = PCAN_USBBUS1
baud                    = PCAN_BAUD_1M
ID_CAN_BOOT             = 0x600
ID_CAN_GO_BOOT          = 0x00A
REQUEST_UPDATE          = 0
SEND_DATA               = 1
SEND_CHEKSUM            = 2
GO_BOOT                 = 3
ALIVE                   = 4



##################################### FUNCIONES ##########################################
# Funcion que lee un fichero binario
def readfile():

    file = open(sys.argv[1], "rb")
    file_binary = file.read()
    file.close()

    sizefile = os.stat(sys.argv[1]).st_size

    print("file binary = ", sys.argv[1])
    print("sizefile = ", sizefile, "bytes")
    print(" ")

    return sizefile, file_binary


# Funcion que inicializa el bus CAN
def can_init():

    result = objPCAN.Initialize(channel, baud)
    if result != PCAN_ERROR_OK:
        result = objPCAN.GetErrorText(result)
        print(result[1])
        exit(0)
    else:
        print("PCAN-USB was initialized")

    # Check the status of the USB Channel
    result = objPCAN.GetStatus(channel)
    if result == PCAN_ERROR_OK:
        print("PCAN_USB: Status is OK")
    else:
        # An error occured, get a text describing the error and show it
        result = objPCAN.GetErrorText(result)
        print(result[1])
        exit(0)

    # Set Message Filter
    result = objPCAN.FilterMessages(channel, 0x000, 0xfff, PCAN_MESSAGE_STANDARD)
    if result != PCAN_ERROR_OK:
        result = objPCAN.GetErrorText(result)
        print(result[1])
        exit(0)
    else:
        print("Message Filter Applied Successfully")


# Funcion que envia una trama CAN
def send_can_frame(id_can, length, *frame):

    ret = 0

    for i in range(length):
        msg.DATA[i] = frame[0][i]

    msg.ID = id_can
    msg.LEN = length

    result = objPCAN.Write(PCAN_USBBUS1, msg)
    if result != PCAN_ERROR_OK:
        # An error occured, get a text describing the error and show it
        result = objPCAN.GetErrorText(result)
        print(result)
    else:
        ret = 1

    return ret


def send_update():

    bytes_sended = 0
    checksum = 0
    timeout = time.time()

    while bytes_sended < sizefile:

        array_data.clear()
        length_frame = 0
        if (sizefile - bytes_sended) >= 8:
            for i in range(0, 8):
                array_data.append(int(file_binary[bytes_sended + i]))
                checksum ^= file_binary[bytes_sended + i]
                length_frame = 8

        else:
            for i in range(0, (sizefile - bytes_sended)):
                array_data.append(int(file_binary[bytes_sended + i]))
                checksum ^= file_binary[bytes_sended + i]
                length_frame = sizefile - bytes_sended

        ret = send_can_frame(ID_CAN_BOOT | SEND_DATA, length_frame, array_data)

        if ret == 1:
            print("Packet ", int(bytes_sended / 8) + 1, " sent successfully ", end='\r')
            bytes_sended += 8
            timeout = time.time()
        else:
            timeout = time.time() - timeout

        time.sleep(0.001)

#    if(timeout > 3000):
#        print("Timeout sending through CAN! ")
#        exit(0)

    return checksum

################################################################################################
##################################### INICIO PROGRAMA ##########################################
################################################################################################


print(" ")
print("Starting Updater...")
print(" ")

n_args = len(sys.argv)
sizefile = 0
objPCAN = PCANBasic()

if n_args == 3:

    # Lee el fichero binario de firmware
    sizefile, file_binary = readfile()
    # Inicializa el bus CAN
    can_init()
    print(" ")
    msg = TPCANMsg()
    msg.MSGTYPE = PCAN_MESSAGE_STANDARD

    time.sleep(1)
    #Pone la ECU en modo Boot (para esperar la actualizacion)
    send_can_frame((ID_CAN_BOOT | GO_BOOT), 1, array_data)

    # Espera a que las ECUs esten en modo BOOT
    time.sleep(1)

    # Envia la peticion de actualizacion junto al tamaño de fichero
    array_data.clear()
    for i in range(0, 8):
        array_data.append(int((sizefile >> 8*i) & 0xFF))

    send_can_frame((ID_CAN_BOOT | REQUEST_UPDATE), 8, array_data)
    time.sleep(1)

    # Envia la actualizacion de firmware y obtiene el checksum calculado
    checksum = send_update()
    time.sleep(1)
    print("\n")

    # Envia el checksum del fichero de firmware
    array_data.clear()
    array_data.append(checksum)
    send_can_frame((ID_CAN_BOOT | SEND_CHEKSUM), 1, array_data)
    time.sleep(1)

    # Recibe de la ECU el resultado de la actualizacion (OK o ERROR)
    msg.LEN = 0
    while msg.LEN == 0:
        readResult = objPCAN.Read(channel)
        msg = readResult[1]  # readResult[1] TPCANMsg()

    if readResult[0] == PCAN_ERROR_OK:

        print('Update finished with status: ', end='')
        for i in range(0, msg.LEN):
            print(chr(msg.DATA[i]), end='')

    objPCAN.Uninitialize(PCAN_NONEBUS)

else:

    print("ERROR: Bad arguments.")
    print("Usage: python + updater.py + file.bin + id_can")
    print("Example: python updater.py firmware.bin 0x321")
    exit(0)



