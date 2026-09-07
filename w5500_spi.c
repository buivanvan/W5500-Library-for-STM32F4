#include "w5500_spi.h"
#include "w5500.h"
#include "stm32f4xx.h"

static SPI_TypeDef *current_SPI;
static GPIO_TypeDef *current_CS_PORT;
static uint8_t current_CS_PIN;

static void W5500_SPI_GPIO_Init(SPI_TypeDef *SPIx)
{
    if (SPIx == SPI1)
    {
        // Enable GPIOA clock
        RCC->AHB1ENR |= (1U << 0);

        // Enable SPI1 clock
        RCC->APB2ENR |= (1U << 12);

        // PA5 = SPI1_SCK, PA6 = SPI1_MISO, PA7 = SPI1_MOSI
        GPIOA->MODER &= ~((3U << (5 * 2)) | (3U << (6 * 2)) | (3U << (7 * 2)));
        GPIOA->MODER |=  ((2U << (5 * 2)) | (2U << (6 * 2)) | (2U << (7 * 2)));

        // Alternate function AF5 for SPI1
        GPIOA->AFR[0] &= ~((0xFU << (5 * 4)) | (0xFU << (6 * 4)) | (0xFU << (7 * 4)));
        GPIOA->AFR[0] |=  ((5U << (5 * 4)) | (5U << (6 * 4)) | (5U << (7 * 4)));
    }
    else if (SPIx == SPI2)
    {
        // Enable GPIOB clock
        RCC->AHB1ENR |= (1U << 1);

        // Enable SPI2 clock
        RCC->APB1ENR |= (1U << 14);

        // PB13 = SPI2_SCK, PB14 = SPI2_MISO, PB15 = SPI2_MOSI
        GPIOB->MODER &= ~((3U << (13 * 2)) | (3U << (14 * 2)) | (3U << (15 * 2)));
        GPIOB->MODER |=  ((2U << (13 * 2)) | (2U << (14 * 2)) | (2U << (15 * 2)));

        // Alternate function AF5 for SPI2
        GPIOB->AFR[1] &= ~((0xFU << ((13 - 8) * 4)) | (0xFU << ((14 - 8) * 4)) | (0xFU << ((15 - 8) * 4)));
        GPIOB->AFR[1] |=  ((5U << ((13 - 8) * 4)) | (5U << ((14 - 8) * 4)) | (5U << ((15 - 8) * 4)));
    }
    else if (SPIx == SPI3)
    {
        // Enable GPIOC clock
        RCC->AHB1ENR |= (1U << 2);

        // Enable SPI3 clock
        RCC->APB1ENR |= (1U << 15);

        // PC10 = SPI3_SCK, PC11 = SPI3_MISO, PC12 = SPI3_MOSI
        GPIOC->MODER &= ~((3U << (10 * 2)) | (3U << (11 * 2)) | (3U << (12 * 2)));
        GPIOC->MODER |=  ((2U << (10 * 2)) | (2U << (11 * 2)) | (2U << (12 * 2)));

        // Alternate function AF6 for SPI3
        GPIOC->AFR[1] &= ~((0xFU << ((10 - 8) * 4)) | (0xFU << ((11 - 8) * 4)) | (0xFU << ((12 - 8) * 4)));
        GPIOC->AFR[1] |=  ((6U << ((10 - 8) * 4)) | (6U << ((11 - 8) * 4)) | (6U << ((12 - 8) * 4)));
    }
}

static void W5500_CS_GPIO_Init(GPIO_TypeDef *cs_port, uint8_t cs_pin)
{
    // Enable clock for selected CS GPIO port
    if (cs_port == GPIOA) RCC->AHB1ENR |= (1U << 0);
    else if (cs_port == GPIOB) RCC->AHB1ENR |= (1U << 1);
    else if (cs_port == GPIOC) RCC->AHB1ENR |= (1U << 2);
    else if (cs_port == GPIOD) RCC->AHB1ENR |= (1U << 3);
    else if (cs_port == GPIOE) RCC->AHB1ENR |= (1U << 4);

    // Configure CS pin as General Purpose Output
    cs_port->MODER &= ~(3U << (cs_pin * 2));
    cs_port->MODER |=  (1U << (cs_pin * 2));

    // Drive CS pin HIGH initially
    cs_port->BSRR = (1U << cs_pin);
}

void W5500_SPI_Init(SPI_TypeDef *SPIx, GPIO_TypeDef *cs_port, uint8_t cs_pin)
{
    // Save selected SPI instance and CS configuration
    current_SPI = SPIx;
    current_CS_PORT = cs_port;
    current_CS_PIN = cs_pin;

    // Configure SPI GPIO pins
    W5500_SPI_GPIO_Init(SPIx);

    // Configure custom CS pin
    W5500_CS_GPIO_Init(cs_port, cs_pin);
	
		// Configure custom Reset pin
		

    // Disable SPI before configuration
    SPIx->CR1 = 0;

    // Set Master mode
    SPIx->CR1 |= (1U << 2);

    // Set Baud rate = PCLK / 8
    SPIx->CR1 |= (2U << 3);

    // Enable Software slave management
    SPIx->CR1 |= (1U << 9);
    SPIx->CR1 |= (1U << 8);

    // Enable SPI peripheral
    SPIx->CR1 |= (1U << 6);
}

uint8_t W5500_SPI_Transfer(uint8_t data)
{
    // Wait until TX buffer is empty
    while (!(current_SPI->SR & SPI_SR_TXE));

    // Send data byte
    *((volatile uint8_t *)&current_SPI->DR) = data;

    // Wait until RX buffer is not empty
    while (!(current_SPI->SR & SPI_SR_RXNE));

    // Return received byte
    return *((volatile uint8_t *)&current_SPI->DR);
}

void W5500_CS_Select(void)
{
    // Drive selected CS pin LOW
    current_CS_PORT->BSRR = (1U << (current_CS_PIN + 16));
}

void W5500_CS_Unselect(void)
{
    // Drive selected CS pin HIGH
    current_CS_PORT->BSRR = (1U << current_CS_PIN);
}

void W5500_Reset(void)
{
    // Drive PB1 RESET pin LOW
    GPIOB->BSRR = (1U << 17);

    // Delay loop
    for (volatile uint32_t i = 0; i < 100000; i++);

    // Drive PB1 RESET pin HIGH
    GPIOB->BSRR = (1U << 1);

    // Delay loop
    for (volatile uint32_t i = 0; i < 100000; i++);
}

void W5500_Init(SPI_TypeDef *SPIx, GPIO_TypeDef *cs_port, uint8_t cs_pin)
{
    // Initialize SPI and CS pin
    W5500_SPI_Init(SPIx, cs_port, cs_pin);

    // Perform hardware reset
    W5500_Reset();

    // Check PHY link status
    uint8_t phycfgr = W5500_ReadByte(W5500_PHYCFGR, W5500_COMMON_REG);
    uint8_t link = phycfgr & 0x01;
    uint8_t timeout = 100;

    while (!link)
    {
        while (timeout--)
        {
            phycfgr = W5500_ReadByte(W5500_PHYCFGR, W5500_COMMON_REG);
            link = phycfgr & 0x01;

            for (int i = 0; i < 5000; i++);
        }
    }
}