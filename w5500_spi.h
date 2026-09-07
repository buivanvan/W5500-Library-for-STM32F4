#ifndef W5500_SPI_H
#define W5500_SPI_H

#include "stm32f4xx.h"
#include <stdint.h>

void W5500_SPI_Init(SPI_TypeDef *SPIx, GPIO_TypeDef *cs_port, uint8_t cs_pin);

uint8_t W5500_SPI_Transfer(uint8_t data);

void W5500_CS_Select(void);
void W5500_CS_Unselect(void);
void W5500_Reset(void);
void W5500_Init(SPI_TypeDef *SPIx, GPIO_TypeDef *cs_port, uint8_t cs_pin);

#endif