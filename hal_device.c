#include "stdint.h"
#include <stddef.h>
#include <string.h>
#include <stdio.h>

/* ==================== 前向声明 ==================== */

typedef struct device device_t;
typedef struct driver driver_t;
typedef struct bus bus_t;
typedef struct spi_bus spi_bus_t;
typedef struct spi_device spi_device_t;
typedef struct spi_driver spi_driver_t;

/* ==================== 基础类型定义 ==================== */

typedef enum {
    DEVICE_TYPE_SPI,
    DEVICE_TYPE_I2C,
    DEVICE_TYPE_GPIO,
    DEVICE_TYPE_UART,
} device_type_t;

typedef enum {
    DRIVER_TYPE_SPI,
    DRIVER_TYPE_I2C,
    DRIVER_TYPE_GPIO,
    DRIVER_TYPE_UART,
} driver_type_t;

/* ==================== 总线定义 ==================== */

struct bus {
    const char *name;
    int (*match)(device_t *dev, driver_t *drv);
    int (*probe)(device_t *dev, driver_t *drv);
    void *private_data;
};

/* ==================== SPI 配置和操作 ==================== */

typedef struct {
    uint32_t max_speed_hz;
    uint8_t mode;           // SPI mode: 0-3
    uint8_t bits_per_word;
    uint8_t cs_pin;
} spi_config_t;

typedef struct spi_ops {
    int (*init)(spi_bus_t *bus);
    int (*deinit)(spi_bus_t *bus);
    int (*transfer)(spi_bus_t *bus, uint8_t *tx, uint8_t *rx, uint16_t len);
    int (*cs_control)(spi_bus_t *bus, uint8_t enable);
    int (*configure)(spi_bus_t *bus, spi_config_t *config);
} spi_ops_t;

/* ==================== SPI 总线 ==================== */

struct spi_bus {
    bus_t bus;
    spi_config_t *config;
    spi_ops_t *ops;
    stm32_spi_private_t *user;  // 具体类型，不再使用void*
};

/* ==================== 设备定义 ==================== */

struct device {
    const char *name;
    device_type_t type;
    bus_t *bus;              // 所属总线，具体类型
    driver_t *driver;        // 绑定的驱动，具体类型
    void *private_data;      // 设备私有数据（保留，因为设备私有数据类型多样）
    uint32_t flags;
    
    // 设备操作函数
    int (*init)(device_t *dev);
    int (*deinit)(device_t *dev);
    int (*read)(device_t *dev, uint8_t *buf, uint32_t len);
    int (*write)(device_t *dev, const uint8_t *buf, uint32_t len);
    int (*ioctl)(device_t *dev, int cmd, void *arg);
    
    device_t *next;          // 链表指针
};

/* ==================== 驱动定义 ==================== */

struct driver {
    const char *name;
    driver_type_t type;
    const char *compatible;  // 兼容的设备名
    
    // 驱动操作函数
    int (*probe)(device_t *dev);      // 设备绑定后调用
    int (*remove)(device_t *dev);     // 设备移除时调用
    int (*suspend)(device_t *dev);    // 电源管理
    int (*resume)(device_t *dev);
    
    void *private_data;
    driver_t *next;          // 链表指针
};

/* ==================== SPI 设备扩展 ==================== */

struct spi_device {
    device_t parent;
    spi_config_t config;
    spi_bus_t *bus;          // 具体类型，不再使用void*
    uint8_t cs_pin_state;
    uint32_t current_speed;
};

/* ==================== SPI 驱动扩展 ==================== */

struct spi_driver {
    driver_t parent;
    int (*spi_transfer)(spi_device_t *dev, uint8_t *tx, uint8_t *rx, uint16_t len);
    int (*spi_configure)(spi_device_t *dev, spi_config_t *config);
};

/* ==================== 硬件私有数据结构 ==================== */

// STM32 SPI 硬件私有数据
typedef struct {
    uint32_t reg_base;
    uint32_t clock;
    int irq_num;
} stm32_spi_private_t;

/* ==================== 核心框架管理 ==================== */

static device_t *device_list_head = NULL;
static driver_t *driver_list_head = NULL;

// 注册设备
int device_register(device_t *dev) {
    if (!dev || !dev->name) return -1;
    
    // 添加到设备链表
    dev->next = device_list_head;
    device_list_head = dev;
    
    // 尝试匹配已注册的驱动
    driver_t *drv = driver_list_head;
    while (drv) {
        if (dev->bus && dev->bus->match) {
            // 调用总线的match函数进行匹配
            if (dev->bus->match(dev, drv)) {
                // 匹配成功，绑定驱动
                dev->driver = drv;
                if (drv->probe) {
                    drv->probe(dev);
                }
                break;
            }
        }
        drv = drv->next;
    }
    
    return 0;
}

// 注册驱动
int driver_register(driver_t *drv) {
    if (!drv || !drv->name) return -1;
    
    // 添加到驱动链表
    drv->next = driver_list_head;
    driver_list_head = drv;
    
    // 尝试匹配已注册的设备
    device_t *dev = device_list_head;
    while (dev) {
        if (dev->bus && dev->bus->match) {
            // 调用总线的match函数进行匹配
            if (dev->bus->match(dev, drv)) {
                // 匹配成功，绑定设备
                dev->driver = drv;
                if (drv->probe) {
                    drv->probe(dev);
                }
                break;
            }
        }
        dev = dev->next;
    }
    
    return 0;
}

// 注销设备
int device_unregister(device_t *dev) {
    if (!dev) return -1;
    
    // 从链表中移除
    device_t **pprev = &device_list_head;
    device_t *curr = device_list_head;
    
    while (curr) {
        if (curr == dev) {
            *pprev = curr->next;
            break;
        }
        pprev = &curr->next;
        curr = curr->next;
    }
    
    // 通知驱动移除设备
    if (dev->driver && dev->driver->remove) {
        dev->driver->remove(dev);
    }
    dev->driver = NULL;
    
    return 0;
}

// 根据名称查找设备
device_t *device_find(const char *name) {
    device_t *dev = device_list_head;
    while (dev) {
        if (strcmp(dev->name, name) == 0) {
            return dev;
        }
        dev = dev->next;
    }
    return NULL;
}

// 根据名称查找驱动
driver_t *driver_find(const char *name) {
    driver_t *drv = driver_list_head;
    while (drv) {
        if (strcmp(drv->name, name) == 0) {
            return drv;
        }
        drv = drv->next;
    }
    return NULL;
}

/* ==================== SPI 总线匹配函数 ==================== */

// SPI 总线匹配函数：判断设备和驱动是否兼容
int spi_bus_match(device_t *device, driver_t *driver) {
    // 检查类型是否匹配
    if (device->type != DEVICE_TYPE_SPI || driver->type != DRIVER_TYPE_SPI) {
        return 0;  // 类型不匹配
    }
    
    // 如果驱动指定了兼容设备名，检查是否匹配
    if (driver->compatible) {
        if (strcmp(device->name, driver->compatible) == 0) {
            return 1;  // 完全匹配
        }
    }
    
    // 默认：设备名包含驱动名即匹配
    if (strstr(device->name, driver->name) != NULL) {
        return 1;
    }
    
    return 0;
}

// SPI 总线探测函数
int spi_bus_probe(device_t *device, driver_t *driver) {
    printf("SPI bus: probing device '%s' with driver '%s'\n", 
           device->name, driver->name);
    
    // 执行具体的设备初始化
    if (device->init) {
        return device->init(device);
    }
    
    return 0;
}

/* ==================== SPI 总线操作实现 ==================== */

// 注册 SPI 总线
int spi_bus_register(spi_bus_t *bus, const char *name, 
                      spi_config_t *config, spi_ops_t *ops) {
    if (!bus || !name || !ops) return -1;
    
    bus->bus.name = name;
    bus->bus.match = spi_bus_match;
    bus->bus.probe = spi_bus_probe;
    bus->config = config;
    bus->ops = ops;
    bus->user = NULL;
    
    // 初始化总线硬件
    if (ops->init) {
        return ops->init(bus);
    }
    
    return 0;
}

// SPI 总线数据传输
int spi_bus_transfer(spi_bus_t *bus, uint8_t *tx, uint8_t *rx, uint16_t len) {
    if (!bus || !bus->ops || !bus->ops->transfer) return -1;
    return bus->ops->transfer(bus, tx, rx, len);
}

// SPI 总线配置
int spi_bus_configure(spi_bus_t *bus, spi_config_t *config) {
    if (!bus || !bus->ops || !bus->ops->configure) return -1;
    return bus->ops->configure(bus, config);
}

// SPI 设备传输
int spi_device_transfer(spi_device_t *dev, uint8_t *tx, uint8_t *rx, uint16_t len) {
    if (!dev || !dev->bus) return -1;
    
    int ret = 0;
    
    // 控制片选
    if (dev->bus->ops->cs_control) {
        dev->bus->ops->cs_control(dev->bus, 1);  // 拉低CS
    }
    
    // 配置SPI参数（如果需要）
    if (dev->current_speed != dev->config.max_speed_hz) {
        spi_bus_configure(dev->bus, &dev->config);
        dev->current_speed = dev->config.max_speed_hz;
    }
    
    // 执行传输
    ret = spi_bus_transfer(dev->bus, tx, rx, len);
    
    // 释放片选
    if (dev->bus->ops->cs_control) {
        dev->bus->ops->cs_control(dev->bus, 0);  // 拉高CS
    }
    
    return ret;
}

// 注册 SPI 设备
int spi_device_register(spi_device_t *spi_dev, const char *name, 
                         spi_bus_t *bus, spi_config_t *config) {
    if (!spi_dev || !bus) return -1;
    
    // 初始化设备结构
    spi_dev->parent.name = name;
    spi_dev->parent.type = DEVICE_TYPE_SPI;
    spi_dev->parent.bus = (bus_t *)bus;
    spi_dev->parent.driver = NULL;
    spi_dev->parent.init = NULL;
    spi_dev->parent.deinit = NULL;
    spi_dev->parent.read = NULL;
    spi_dev->parent.write = NULL;
    spi_dev->parent.ioctl = NULL;
    
    spi_dev->bus = bus;
    if (config) {
        spi_dev->config = *config;
        spi_dev->current_speed = 0;
    }
    spi_dev->cs_pin_state = 0;
    
    // 注册到设备框架
    return device_register((device_t *)spi_dev);
}

/* ==================== 硬件底层实现 ==================== */

// STM32 SPI 硬件操作
static int stm32_spi_init(spi_bus_t *bus) {
    stm32_spi_private_t *priv = bus->user;
    
    if (!priv) return -1;
    
    // 使能SPI时钟
    // RCC->APB2ENR |= (1 << priv->clock);
    
    // 配置SPI参数
    // SPIx->CR1 = bus->config->mode;
    // SPIx->CR2 = bus->config->bits_per_word;
    
    // 使能SPI
    // SPIx->CR1 |= (1 << 6);
    
    printf("STM32 SPI%d initialized (base=0x%X)\n", 
           priv->irq_num, priv->reg_base);
    
    return 0;
}

static int stm32_spi_transfer(spi_bus_t *bus, uint8_t *tx, uint8_t *rx, uint16_t len) {
    stm32_spi_private_t *priv = bus->user;
    
    if (!priv) return -1;
    
    for (uint16_t i = 0; i < len; i++) {
        // 等待发送缓冲区空
        // while (!(SPIx->SR & (1 << 1)));
        
        // 发送数据
        // *(volatile uint8_t *)&SPIx->DR = tx ? tx[i] : 0;
        
        // 等待接收完成
        // while (!(SPIx->SR & (1 << 0)));
        
        // 读取数据
        // if (rx) rx[i] = *(volatile uint8_t *)&SPIx->DR;
        
        // 模拟数据传输
        if (rx) rx[i] = tx ? tx[i] : 0xFF;
    }
    
    return 0;
}

static int stm32_spi_configure(spi_bus_t *bus, spi_config_t *config) {
    stm32_spi_private_t *priv = bus->user;
    
    if (!priv) return -1;
    
    printf("STM32 SPI: config mode=%d, speed=%d Hz, bits=%d\n",
           config->mode, config->max_speed_hz, config->bits_per_word);
    
    // 实际配置硬件寄存器
    // SPIx->CR1 = config->mode;
    // SPIx->CR2 = config->bits_per_word;
    // SPIx->BAUDRATE = calculate_baudrate(config->max_speed_hz);
    
    return 0;
}

static spi_ops_t stm32_spi_ops = {
    .init = stm32_spi_init,
    .deinit = NULL,
    .transfer = stm32_spi_transfer,
    .cs_control = NULL,  // 硬件自动CS或单独实现
    .configure = stm32_spi_configure,
};

/* ==================== SPI 设备驱动示例 ==================== */

// W25Q64 Flash 驱动
static int w25q64_probe(device_t *dev) {
    spi_device_t *spi_dev = (spi_device_t *)dev;
    
    printf("W25Q64 Flash probed on SPI bus\n");
    
    // 读取设备ID验证
    uint8_t tx_buf[] = {0x9F};  // Read ID命令
    uint8_t rx_buf[3] = {0};
    
    if (spi_device_transfer(spi_dev, tx_buf, rx_buf, 1) == 0) {
        printf("W25Q64 Device ID: %02X %02X %02X\n", 
               rx_buf[0], rx_buf[1], rx_buf[2]);
    }
    
    return 0;
}

static int w25q64_read(device_t *dev, uint8_t *buf, uint32_t len) {
    spi_device_t *spi_dev = (spi_device_t *)dev;
    // 实现Flash读操作
    return spi_device_transfer(spi_dev, NULL, buf, len);
}

static int w25q64_write(device_t *dev, const uint8_t *buf, uint32_t len) {
    spi_device_t *spi_dev = (spi_device_t *)dev;
    // 实现Flash写操作
    return spi_device_transfer(spi_dev, (uint8_t*)buf, NULL, len);
}

static spi_driver_t w25q64_driver = {
    .parent = {
        .name = "w25q64",
        .type = DRIVER_TYPE_SPI,
        .compatible = "winbond,w25q64",
        .probe = w25q64_probe,
        .remove = NULL,
        .suspend = NULL,
        .resume = NULL,
    },
    .spi_transfer = NULL,  // 使用标准spi_device_transfer
    .spi_configure = NULL,
};

/* ==================== 应用示例 ==================== */

int main(void) {
    // 1. 创建SPI总线
    static spi_bus_t spi1_bus;
    static spi_config_t spi1_config = {
        .max_speed_hz = 1000000,
        .mode = 0,
        .bits_per_word = 8,
        .cs_pin = 10,
    };
    static stm32_spi_private_t spi1_priv = {
        .reg_base = 0x40013000,  // SPI1基址
        .clock = 12,
        .irq_num = 35,
    };
    
    spi1_bus.user = &spi1_priv;
    spi_bus_register(&spi1_bus, "spi1", &spi1_config, &stm32_spi_ops);
    printf("SPI bus registered: %s\n", spi1_bus.bus.name);
    
    // 2. 创建SPI设备（W25Q64 Flash）
    static spi_device_t w25q64_device;
    static spi_config_t w25q64_config = {
        .max_speed_hz = 5000000,  // Flash支持更高速度
        .mode = 0,
        .bits_per_word = 8,
        .cs_pin = 10,
    };
    
    spi_device_register(&w25q64_device, "w25q64", &spi1_bus, &w25q64_config);
    printf("SPI device registered: %s\n", w25q64_device.parent.name);
    
    // 3. 注册驱动
    driver_register((driver_t *)&w25q64_driver);
    printf("Driver registered: %s\n", w25q64_driver.parent.name);
    
    // 4. 使用设备
    device_t *found_dev = device_find("w25q64");
    if (found_dev) {
        uint8_t read_buf[256];
        // 读取Flash数据
        // found_dev->read(found_dev, read_buf, 256);
        printf("Device found and ready to use\n");
    }
    
    // 5. 执行SPI传输示例
    uint8_t tx_data[] = {0x01, 0x02, 0x03, 0x04};
    uint8_t rx_data[4];
    
    spi_device_transfer(&w25q64_device, tx_data, rx_data, 4);
    printf("SPI transfer completed\n");
    
    return 0;
}
