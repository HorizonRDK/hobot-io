import os
import os.path
import copy


HOBOT_PI = 'HOBOT_PI'
SYSFS_GPIO = "/sys/class/gpio"
SYSFS_PLATFORM_PATH = '/sys/devices/platform/'
SYSFS_BOARDID_PATH = '/sys/class/socinfo/board_id'
HOBOT_PI_PATTERN = 'hobot,x3'

# [0]- GPIO chip sysfs directory
# [1]- Linux exported GPIO name,
# [2]- Pin number (BOARD mode)
# [3]- Pin number (BCM mode)
# [4]- Pin name (CVM mode)
# [5]- Pin name (SOC mode)
# [6]- PWM chip sysfs directory
# [7]- PWM ID

SDBV3_PIN = [
    ["soc/a6003000.gpio", 11, 3, 2, 'I2C1_SDA', 'I2C1_SDA', None, None],
    ["soc/a6003000.gpio", 10, 5, 3, 'I2C1_SCL', 'I2C1_SCL', None, None],
    ["soc/a6003000.gpio", 38, 7, 4, 'GPIO38', 'GPIO38', None, None],
    ["soc/a6003000.gpio", 6, 11, 17, 'GPIO6', 'JTG_TDI', None, None],
    ["soc/a6003000.gpio", 5, 13, 27, 'GPIO5', 'JTG_TMS', None, None],
    ["soc/a6003000.gpio", 30, 15, 22, 'GPIO30', 'BIFSPI_MISO', None, None],
    ["soc/a6003000.gpio", 18, 19, 10, 'SPI0_MOSI', 'SPI0_MOSI', "soc/a500e000.pwm", 2],
    ["soc/a6003000.gpio", 19, 21, 9, 'SPI0_MISO', 'SPI0_MISO', "soc/a500f000.pwm", 0],
    ["soc/a6003000.gpio", 17, 23, 11, 'SPI0_SCLK', 'SPI0_SCLK', None, None],
    ["soc/a6003000.gpio", 14, 29, 5, 'GPIO14', 'SPI2_SCLK', None, None],
    ["soc/a6003000.gpio", 31, 31, 6, 'GPIO31', 'BIFSPI_RSTN', None, None],
    ["soc/a6003000.gpio", 13, 33, 13, 'PWM8', 'SPI2_MISO', "soc/a500f000.pwm", 2],
    ["soc/a6003000.gpio", 103, 35, 19, 'I2S0_LRCK', 'I2S0_LRCK', None, None],
    ["soc/a6003000.gpio", 29, 37, 26, 'GPIO29', 'BIFSPI_MOSI', None, None],

    ["soc/a6003000.gpio", 95, 8, 14, 'UART0_TXD', 'UART0_TXD', None, None],
    ["soc/a6003000.gpio", 96, 10, 15,'UART0_RXD', 'UART0_RXD', None, None],
    ["soc/a6003000.gpio", 102, 12, 18, 'I2S0_BCLK', 'I2S0_BCLK', None, None],
    ["soc/a6003000.gpio", 27, 16, 23, 'GPIO27', 'BIFSPI_CSN', None, None],
    ["soc/a6003000.gpio", 7, 18, 24, 'GPIO7', 'JTG_TDO', None, None],
    ["soc/a6003000.gpio", 15, 22, 25, 'GPIO15', 'SPI2_CSN', None, None],
    ["soc/a6003000.gpio", 16, 24, 8, 'SPI0_CSN', 'I2C4_SDA', None, None],
    ["soc/a6003000.gpio", 120, 26, 7, 'SPI0_CSN1', 'QSPI_CSN1', None, None],
    ["soc/a6003000.gpio", 12, 32, 12, 'PWM7', 'SPI2_MOSI', "soc/a500f000.pwm", 1],
    ["soc/a6003000.gpio", 28, 36, 16, 'GPIO28', 'BIFSPI_SCLK', None, None],
    ["soc/a6003000.gpio", 104, 38, 20, 'I2S0_SDIO', 'I2S0_SDIO', None, None],
    ["soc/a6003000.gpio", 108, 40, 21, 'I2S1_SDIO', 'I2S1_SDIO', None, None],
]

SDB_PIN = [
    ["soc/a6003000.gpio", 11, 3, 2, 'I2C1_SDA', 'I2C1_SDA', None, None],
    ["soc/a6003000.gpio", 10, 5, 3, 'I2C1_SCL', 'I2C1_SCL', None, None],
    ["soc/a6003000.gpio", 101, 7, 4, 'I2S0_MCLK', 'I2S0_MCLK', None, None],
    ["soc/a6003000.gpio", 6, 11, 17, 'GPIO6', 'JTG_TDI', None, None],
    ["soc/a6003000.gpio", 5, 13, 27, 'GPIO5', 'JTG_TMS', None, None],
    ["soc/a6003000.gpio", 30, 15, 22, 'GPIO30', 'BIFSPI_MISO', None, None],
    ["soc/a6003000.gpio", 18, 19, 10, 'SPI0_MOSI', 'SPI0_MOSI', "soc/a500e000.pwm", 2],
    ["soc/a6003000.gpio", 19, 21, 9, 'SPI0_MISO', 'SPI0_MISO', "soc/a500f000.pwm", 0],
    ["soc/a6003000.gpio", 17, 23, 11, 'SPI0_SCLK', 'SPI0_SCLK', None, None],
    ["soc/a6003000.gpio", 106, 27, 0, 'I2S1_BLCK', 'I2S1_BLCK', None, None],
    ["soc/a6003000.gpio", 14, 29, 5, 'GPIO14', 'SPI2_SCLK', None, None],
    ["soc/a6003000.gpio", 31, 31, 6, 'GPIO31', 'BIFSPI_RSTN', None, None],
    ["soc/a6003000.gpio", 4, 33, 13, 'PWM0', 'PWM0', "soc/a500d000.pwm", 0],
    ["soc/a6003000.gpio", 103, 35, 19, 'I2S0_LRCK', 'I2S0_LRCK', None, None],
    ["soc/a6003000.gpio", 105, 37, 26, 'GPIO105', 'I2S1_MCLK', None, None],

    ["soc/a6003000.gpio", 111, 8, 14, 'UART3_TXD', 'UART3_TXD', None, None],
    ["soc/a6003000.gpio", 112, 10, 15,'UART3_RXD', 'UART3_RXD', None, None],
    ["soc/a6003000.gpio", 102, 12, 18, 'I2S0_BCLK', 'I2S0_BCLK', None, None],
    ["soc/a6003000.gpio", 27, 16, 23, 'GPIO27', 'BIFSPI_CSN', None, None],
    ["soc/a6003000.gpio", 7, 18, 24, 'GPIO7', 'JTG_TDO', None, None],
    ["soc/a6003000.gpio", 15, 22, 25, 'GPIO15', 'SPI2_CSN', None, None],
    ["soc/a6003000.gpio", 16, 24, 8, 'SPI0_CSN', 'I2C4_SDA', None, None],
    ["soc/a6003000.gpio", 120, 26, 7, 'SPI0_CSN1', 'QSPI_CSN1', None, None],
    ["soc/a6003000.gpio", 107, 28, 1, 'I2S1_LRCK', 'I2S1_LRCK', None, None],
    ["soc/a6003000.gpio", 25, 32, 12, 'PWM4', 'PWM4', "soc/a500e000.pwm", 1],
    ["soc/a6003000.gpio", 3, 36, 16, 'GPIO3', 'JTG_TCK', None, None],
    ["soc/a6003000.gpio", 104, 38, 20, 'I2S0_SDIO', 'I2S0_SDIO', None, None],
    ["soc/a6003000.gpio", 108, 40, 21, 'I2S1_SDIO', 'I2S1_SDIO', None, None],
]

X3_PI_PIN = [
    ["soc/a6003000.gpio", 9, 3, 2, 'I2C0_SDA', 'I2C0_SDA', None, None],
    ["soc/a6003000.gpio", 8, 5, 3,'I2C0_SCL', 'I2C0_SCL', None, None],
    ["soc/a6003000.gpio", 101, 7, 4, 'I2S0_MCLK', 'I2S0_MCLK', None, None],
    ["soc/a6003000.gpio", 6, 11, 17, 'GPIO6', 'JTG_TDI', None, None],
    ["soc/a6003000.gpio", 5, 13, 27,'GPIO5', 'JTG_TMS', None, None],
    ["soc/a6003000.gpio", 30, 15, 22, 'GPIO30', 'BIFSPI_MISO', None, None],
    ["soc/a6003000.gpio", 12, 19, 10, 'SPI2_MOSI', 'SPI2_MOSI', "soc/a500f000.pwm", 1],
    ["soc/a6003000.gpio", 13, 21, 9, 'SPI2_MISO', 'SPI2_MISO', "soc/a500f000.pwm", 2],
    ["soc/a6003000.gpio", 14, 23, 11, 'SPI2_SCLK', 'SPI2_SCLK', None, None],
    ["soc/a6003000.gpio", 106, 27, 0, 'I2S1_BCLK', 'I2S1_BCLK', None, None],
    ["soc/a6003000.gpio", 119, 29, 5, 'GPIO119', 'GPIO119', None, None],
    ["soc/a6003000.gpio", 118, 31, 6, 'GPIO118', 'GPIO118', None, None],
    ["soc/a6003000.gpio", 4, 33, 13, 'PWM0', 'JTG_TRSTN', "soc/a500d000.pwm", 0],
    ["soc/a6003000.gpio", 103, 35, 19, 'I2S0_LRCK', 'I2S0_LRCK', None, None],
    ["soc/a6003000.gpio", 105, 37, 26, 'GPIO105', 'I2S1_MCLK', None, None],

    ["soc/a6003000.gpio", 111, 8, 14, 'UART_TXD', 'SENSOR2_MCLK', None, None],
    ["soc/a6003000.gpio", 112, 10, 15, 'UART_RXD', 'SENSOR3_MCLK', None, None],
    ["soc/a6003000.gpio", 102, 12, 18, 'I2S0_BCLK', 'I2S0_BCLK', None, None],
    ["soc/a6003000.gpio", 27, 16, 23, 'GPIO27', 'BIFSPI_CSN', None, None],
    ["soc/a6003000.gpio", 7, 18, 24, 'GPIO7', 'JTG_TDO', None, None],
    ["soc/a6003000.gpio", 29, 22, 25, 'GPIO29', 'BIFSPI_MOSI', None, None],
    ["soc/a6003000.gpio", 15, 24, 8, 'SPI2_CSN', 'SPI2_CSN', None, None],
    ["soc/a6003000.gpio", 28, 26, 7, 'GPIO28', 'BIFSPI_SCLK', None, None],
    ["soc/a6003000.gpio", 107, 28, 1, 'I2S1_LRCK', 'I2S1_LRCK', None, None],
    ["soc/a6003000.gpio", 25, 32, 12, 'PWM4', 'PWM4', "soc/a500e000.pwm", 1],
    ["soc/a6003000.gpio", 3, 36, 16, 'GPIO3', 'JTG_TCK', None, None],
    ["soc/a6003000.gpio", 104, 38, 20, 'I2S0_SDIO', 'I2S0_SDIO', None, None],
    ["soc/a6003000.gpio", 108, 40, 21, 'I2S1_SDIO', 'I2S1_SDIO', None, None],
]

X3_CM_PIN = [
    ["soc/a6003000.gpio", 9, 3, 2, 'I2C0_SDA', 'I2C0_SDA', None, None],
    ["soc/a6003000.gpio", 8, 5, 3,'I2C0_SCL', 'I2C0_SCL', None, None],
    ["soc/a6003000.gpio", 101, 7, 4, 'I2S0_MCLK', 'I2S0_MCLK', None, None],
    ["soc/a6003000.gpio", 12, 11, 17, 'GPIO17', 'SPI2_MOSI', "soc/a500f000.pwm", 1],
    ["soc/a6003000.gpio", 13, 13, 27,'GPIO27', 'SPI2_MISO', "soc/a500f000.pwm", 2],
    ["soc/a6003000.gpio", 30, 15, 22, 'GPIO22', 'BIFSPI_MISO', None, None],
    ["soc/a6003000.gpio", 6, 19, 10, 'SPI1_MOSI', 'SPI1_MOSI', None, None],
    ["soc/a6003000.gpio", 7, 21, 9, 'SPI1_MISO', 'SPI1_MISO', None, None],
    ["soc/a6003000.gpio", 3, 23, 11, 'SPI1_SCLK', 'SPI1_SCLK', None, None],
    ["soc/a6003000.gpio", 15, 27, 0, 'I2C3_SDA', 'I2C3_SDA', None, None],
    ["soc/a6003000.gpio", 119, 29, 5, 'GPIO5', 'LPWM3', None, None],
    ["soc/a6003000.gpio", 118, 31, 6, 'GPIO6', 'LPWM4', None, None],
    ["soc/a6003000.gpio", 4, 33, 13, 'PWM0', 'PWM0', "soc/a500d000.pwm", 0],
    ["soc/a6003000.gpio", 103, 35, 19, 'I2S0_LRCK', 'I2S0_LRCK', None, None],
    ["soc/a6003000.gpio", 117, 37, 26, 'GPIO25', 'LPWM5', None, None],

    ["soc/a6003000.gpio", 111, 8, 14, 'UART_TXD', 'UART3_TXD', None, None],
    ["soc/a6003000.gpio", 112, 10, 15, 'UART_RXD', 'UART3_RXD', None, None],
    ["soc/a6003000.gpio", 102, 12, 18, 'I2S0_BCLK', 'I2S0_BCLK', None, None],
    ["soc/a6003000.gpio", 27, 16, 23, 'GPIO23', 'BIFSPI_CSN', None, None],
    ["soc/a6003000.gpio", 22, 18, 24, 'GPIO24', 'PWM1', "soc/a500d000.pwm", 1],
    ["soc/a6003000.gpio", 29, 22, 25, 'GPIO25', 'BIFSPI_MOSI', None, None],
    ["soc/a6003000.gpio", 5, 24, 8, 'SPI1_CSN', 'SPI1_CSN', None, None],
    ["soc/a6003000.gpio", 28, 26, 7, 'GPIO7', 'BIFSPI_SCLK', None, None],
    ["soc/a6003000.gpio", 14, 28, 1, 'I2C3_SCL', 'I2C3_SCL', None, None],
    ["soc/a6003000.gpio", 25, 32, 12, 'PWM4', 'PWM4', "soc/a500e000.pwm", 1],
    ["soc/a6003000.gpio", 20, 36, 16, 'GPIO16', 'BIFSD_CLK', None, None],
    ["soc/a6003000.gpio", 108, 38, 20, 'I2S1_SDIO', 'I2S1_SDIO', None, None],    
    ["soc/a6003000.gpio", 104, 40, 21, 'I2S0_SDIO', 'I2S0_SDIO', None, None],
]

X3_PI_V2_1_PIN = [
    ["soc/a6003000.gpio", 9, 3, 2, 'I2C0_SDA', 'I2C0_SDA', None, None],
    ["soc/a6003000.gpio", 8, 5, 3,'I2C0_SCL', 'I2C0_SCL', None, None],
    ["soc/a6003000.gpio", 101, 7, 4, 'I2S0_MCLK', 'I2S0_MCLK', None, None],
    ["soc/a6003000.gpio", 12, 11, 17, 'GPIO17', 'SPI2_MOSI', "soc/a500f000.pwm", 1],
    ["soc/a6003000.gpio", 13, 13, 27,'GPIO27', 'SPI2_MISO', "soc/a500f000.pwm", 2],
    ["soc/a6003000.gpio", 30, 15, 22, 'GPIO22', 'BIFSPI_MISO', None, None],
    ["soc/a6003000.gpio", 6, 19, 10, 'SPI1_MOSI', 'SPI1_MOSI', None, None],
    ["soc/a6003000.gpio", 7, 21, 9, 'SPI1_MISO', 'SPI1_MISO', None, None],
    ["soc/a6003000.gpio", 3, 23, 11, 'SPI1_SCLK', 'SPI1_SCLK', None, None],
    ["soc/a6003000.gpio", 15, 27, 0, 'I2C3_SDA', 'I2C3_SDA', None, None],
    ["soc/a6003000.gpio", 119, 29, 5, 'GPIO5', 'LPWM3', None, None],
    ["soc/a6003000.gpio", 118, 31, 6, 'GPIO6', 'LPWM4', None, None],
    ["soc/a6003000.gpio", 4, 33, 13, 'PWM0', 'PWM0', "soc/a500d000.pwm", 0],
    ["soc/a6003000.gpio", 103, 35, 19, 'I2S0_LRCK', 'I2S0_LRCK', None, None],
    ["soc/a6003000.gpio", 117, 37, 26, 'GPIO25', 'LPWM5', None, None],

    ["soc/a6003000.gpio", 111, 8, 14, 'UART_TXD', 'UART3_TXD', None, None],
    ["soc/a6003000.gpio", 112, 10, 15, 'UART_RXD', 'UART3_RXD', None, None],
    ["soc/a6003000.gpio", 102, 12, 18, 'I2S0_BCLK', 'I2S0_BCLK', None, None],
    ["soc/a6003000.gpio", 27, 16, 23, 'GPIO23', 'BIFSPI_CSN', None, None],
    ["soc/a6003000.gpio", 22, 18, 24, 'GPIO24', 'PWM1', "soc/a500d000.pwm", 1],
    ["soc/a6003000.gpio", 29, 22, 25, 'GPIO25', 'BIFSPI_MOSI', None, None],
    ["soc/a6003000.gpio", 5, 24, 8, 'SPI1_CSN', 'SPI1_CSN', None, None],
    ["soc/a6003000.gpio", 28, 26, 7, 'GPIO7', 'BIFSPI_SCLK', None, None],
    ["soc/a6003000.gpio", 14, 28, 1, 'I2C3_SCL', 'I2C3_SCL', None, None],
    ["soc/a6003000.gpio", 25, 32, 12, 'PWM4', 'PWM4', "soc/a500e000.pwm", 1],
    ["soc/a6003000.gpio", 20, 36, 16, 'GPIO16', 'BIFSD_CLK', None, None],
    ["soc/a6003000.gpio", 104, 38, 20, 'I2S0_SDIO', 'I2S0_SDIO', None, None],
    ["soc/a6003000.gpio", 108, 40, 21, 'I2S1_SDIO', 'I2S1_SDIO', None, None],
]

ALL_BOARD_DATA = [
    {'board_name' : 'X3SDBV3', 'pin_info' : SDBV3_PIN, 'board_id' : 0x304},
    {'board_name' : 'X3SDB', 'pin_info' : SDB_PIN, 'board_id' : 0x404},
    {'board_name' : 'X3PI', 'pin_info' : X3_PI_PIN, 'board_id' : 0x504},
    {'board_name' : 'X3PI', 'pin_info' : X3_PI_PIN, 'board_id' : 0x604},
    {'board_name' : 'X3CM', 'pin_info' : X3_CM_PIN, 'board_id' : 0xb04},
    {'board_name' : 'X3PI_V2_1', 'pin_info' : X3_PI_V2_1_PIN, 'board_id' : 0x804}
]


class AllInfo(object):
    def __init__(self, gpio_chip_dir, gpio_id, pwm_chip_dir, pwm_id):
        self.gpio_chip_dir = gpio_chip_dir
        self.gpio_id = gpio_id
        self.pwm_chip_dir = pwm_chip_dir
        self.pwm_id = pwm_id


def get_all_pin_data():
    if (not os.access(SYSFS_GPIO + '/export', os.W_OK)):
        raise RuntimeError("Insufficient permissions, need root permissions:" + SYSFS_GPIO + '/export')
    if (not os.access(SYSFS_GPIO + '/unexport', os.W_OK)):
        raise RuntimeError("Insufficient permissions, need root permissions")
    if (not os.access(SYSFS_BOARDID_PATH, os.R_OK)):
        raise RuntimeError("Insufficient permissions, need root permissions")
    if (not os.access(SYSFS_BOARDID_PATH, os.R_OK)):
        raise RuntimeError("Insufficient permissions, need root permissions")

    res = False
    compatible_path = '/proc/device-tree/compatible'
    with open(compatible_path, 'r') as f:
        compatibles = f.read().split('\x00')
    with open(SYSFS_BOARDID_PATH, 'r') as f:
        sboard_id = "0x" + f.read()
        iboard_id = int(sboard_id,16)
        board_id = iboard_id & 0xfff
    for board_data in ALL_BOARD_DATA:
        if board_data['board_id'] == board_id:
            pin_data = copy.deepcopy(board_data['pin_info'])
            model = board_data['board_name']
            res = True
            break
    if not res:
        raise Exception("Board type is not support")
    # Check GPIO chip dir
    for n, x in enumerate(pin_data):
        if x[0] is None:
            continue
        gpio_chip_name = x[0]
        gpio_chip_dir = SYSFS_PLATFORM_PATH + gpio_chip_name
        gpio_chip_dir = gpio_chip_dir + '/gpio'
        if not os.path.exists(gpio_chip_dir):
            continue
        for f in os.listdir(gpio_chip_dir):
            if not f.startswith('gpiochip'):
                continue
            gpio_chip_dir = gpio_chip_dir + '/' + f
            pin_data[n][0] = gpio_chip_dir
            break
    # Check PWM chip dir
    for n, x in enumerate(pin_data):
        if x[6] is None:
            continue
        pwm_chip_name = x[6]
        pwm_chip_dir = SYSFS_PLATFORM_PATH + pwm_chip_name
        pwm_chip_dir = pwm_chip_dir + '/pwm'
        if not os.path.exists(pwm_chip_dir):
            continue
        for f in os.listdir(pwm_chip_dir):
            if not f.startswith('pwmchip'):
                continue
            pwm_chip_dir = pwm_chip_dir + '/' + f
            pin_data[n][6] = pwm_chip_dir
            break

    def sort_data(pin_name, pin_data):
        return {x[pin_name]: AllInfo(
            x[0],
            x[1],
            x[6],
            x[7]) for x in pin_data}

    all_pin_data = {
        'BOARD': sort_data(2, pin_data),
        'BCM': sort_data(3, pin_data),
        'CVM': sort_data(4, pin_data),
        'SOC': sort_data(1, pin_data),
        'TEGRA_SOC': sort_data(5, pin_data),
    }
    return model, all_pin_data
