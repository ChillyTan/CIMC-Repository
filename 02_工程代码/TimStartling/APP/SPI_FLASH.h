/************************************************************
 * 版权：2025CIMC Copyright。 
 * 文件：SPI_Flash.h
 * 作者: Jianchuan Wang & Tao Liu @ GigaDevice
 * 平台: 2025CIMC IHD-V04
 * 版本: Jianchuan Wang     2025/4/20     V0.01    original
************************************************************/

#ifndef __SPI_FLASH_H
#define __SPI_FLASH_H

/************************* 头文件 *************************/

#include "bsp_system.h"

#define  SPI_FLASH_PAGE_SIZE           256
#define  SPI_FLASH_SECTOR_SIZE         4096
#define  SPI_FLASH_CS_LOW()            gpio_bit_reset(GPIOB, GPIO_PIN_12)
#define  SPI_FLASH_CS_HIGH()           gpio_bit_set(GPIOB, GPIO_PIN_12)


void spi_flash_init(void);                          //初始化SPI Flash
uint32_t spi_flash_read_id(void);                         //读取芯片ID


//SPI最底层操作
uint8_t spi_flash_send_byte(uint8_t byte);                //发送指定字节
uint16_t spi_flash_send_halfword(uint16_t half_word);     //发送指定半字

//写入操作(不可以覆盖写)
void spi_flash_write_enable(void);                                                            //使能写访问
void spi_flash_page_write(uint8_t* pbuffer,uint32_t write_addr,uint16_t num_byte_to_write);   //写入指定页
void spi_flash_buffer_write(uint8_t* pbuffer,uint32_t write_addr,uint32_t num_byte_to_write); //写入指定缓冲区
void spi_flash_wait_for_write_end(void);                                                      //等待写入完成

//读取操作
void spi_flash_start_read_sequence(uint32_t read_addr);   //开始读取指定地址
uint8_t spi_flash_read_byte(void);                        //读取指定地址的字节
void spi_flash_buffer_read(uint8_t* pbuffer,uint32_t read_addr,uint16_t num_byte_to_read);  //读取指定缓冲区

//擦除最小单位为一个最小单位为一个扇区4KB)
void spi_flash_sector_erase(uint32_t sector_addr);                                //擦除指定扇区
void spi_flash_buffer_erase(uint32_t sector_addr, uint32_t num_byte_to_erase);    //擦除指定缓冲区
void spi_flash_bulk_erase(void);                                                  //擦除整个芯片

#endif
