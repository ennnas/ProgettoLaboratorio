#include <stm32f10x.h>
// Reset and clock control 
#include <stm32f10x_rcc.h>
// General purpose I/O
#include <stm32f10x_gpio.h>
// I2C Serial comunication
#include <stm32f10x_i2c.h>
// USART Serial comunication
#include <stm32f10x_usart.h>
// External interrupt 
#include <stm32f10x_exti.h>
// NVIC
#include <misc.h>

#define I2Cx_RCC     	RCC_APB1Periph_I2C2
#define I2Cx         	I2C2
#define I2C_GPIO_RCC    RCC_APB2Periph_GPIOB
#define I2C_GPIO        GPIOB
// Pin su cui sono mappate le SDA e SCL
#define I2C_PIN_SDA     GPIO_Pin_11
#define I2C_PIN_SCL     GPIO_Pin_10

// Indirizzo dell'Arduino
#define SLAVE_ADDRESS       0x08

void i2c_init(void);
void i2c_start(void);
void i2c_stop(void);
void i2c_address_direction(uint8_t address, uint8_t direction);
void i2c_transmit(uint8_t byte);
uint8_t i2c_receive_ack(void);
uint8_t i2c_receive_nack(void);
void i2c_write(uint8_t address, uint8_t data);
void i2c_read(uint8_t address, uint8_t* data);
void Delay(uint32_t nTime);
int uart_open(uint32_t baud, uint32_t flags);
int uart_putc(int c, USART_TypeDef* USARTx);
int uart_getc(USART_TypeDef* USARTx);
void EXTI15_10_IRQHandler(void);

// variabili globali per il blink del led
static int ledval = 0;
// silenzia il blinking del led se a 0
int button = 1;

// byte ricevuto da Arduino
uint8_t receivedByte;
    
int main(void)
{
	// USART full-duplex a velocità 19200
    uint32_t mode  = USART_Mode_Rx | USART_Mode_Tx; 
    uint32_t baud = 19200;
    GPIO_InitTypeDef GPIO_InitStructure;
    char *Start = "\nInizio il setup di STM32\n";
    char *Finish = "Setup completato, sveglio Arduino\n";
    char *c;

    // Abilito i bus A e C che useremo per il buttono, il led e la USART2
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOA, ENABLE); // (1)
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    // Inizia la comunicazione con la seriale
    uart_open(baud,mode);
    c = Start;
    while(*c){
        uart_putc(*(c++),USART2);
    }

    GPIO_StructInit(&GPIO_InitStructure); 
    // PC7 (LED)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7; 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; 
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    // PC13 (Button)
    GPIO_StructInit(&GPIO_InitStructure); 
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13; 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    // Generating a tick evevent every 10-3 sec
    if (SysTick_Config(SystemCoreClock / 1000)) while (1);

    // Interrupts ////////////////////////////////////

    //enable AFIO clock
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,  ENABLE);

    // Connect EXTI13 to PC13
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource13);

    // Select and configure line13 
    EXTI_InitTypeDef  EXTI_InitStructure;

    EXTI_InitStructure.EXTI_Line = EXTI_Line13;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; 
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; 
    EXTI_InitStructure.EXTI_LineCmd = ENABLE; 
    EXTI_Init(&EXTI_InitStructure);

    //disable AFIO clock
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,  DISABLE);

    // NVIC
    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_InitStructure.NVIC_IRQChannel =  EXTI15_10_IRQn;
    // 0 bit priorità 4 bit sottopriorità
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0); 
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3; 
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; 
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
    NVIC_Init(&NVIC_InitStructure);
    ////////////////////////////////////////////////////////////
    
    c = Finish;
    while(*c){
        uart_putc(*(c++),USART2);
    }
    // Initialize I2C
    i2c_init();
    i2c_write(SLAVE_ADDRESS, 0x01);

    while (1)
    {
        i2c_read(SLAVE_ADDRESS, &receivedByte);
        //GPIO_WriteBit(GPIOA, GPIO_Pin_5, (ledval) ? Bit_SET : Bit_RESET); 
        if(receivedByte==1){
            while(button){
                ledval = 1 - ledval;
                GPIO_WriteBit(GPIOC, GPIO_Pin_7, (ledval) ? Bit_SET:Bit_RESET);
                Delay(250);
            }
            GPIO_WriteBit(GPIOC, GPIO_Pin_7, Bit_RESET);
        }
        Delay(1000);
    }
}

void i2c_init()
{
    // Initialization struct
    I2C_InitTypeDef I2C_InitStruct;
    GPIO_InitTypeDef GPIO_InitStruct;
    
    // Step 1: Initialize I2C
    RCC_APB1PeriphClockCmd(I2Cx_RCC, ENABLE);
    I2C_InitStruct.I2C_ClockSpeed = 100000;
    I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStruct.I2C_OwnAddress1 = 0x00;
    I2C_InitStruct.I2C_Ack = I2C_Ack_Disable;
    I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_Init(I2Cx, &I2C_InitStruct);
    I2C_Cmd(I2Cx, ENABLE);
    
    // Step 2: Initialize GPIO as open drain alternate function
    RCC_APB2PeriphClockCmd(I2C_GPIO_RCC, ENABLE);
    GPIO_InitStruct.GPIO_Pin = I2C_PIN_SCL | I2C_PIN_SDA;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(I2C_GPIO, &GPIO_InitStruct);
}

void i2c_start()
{
    // Wait until I2Cx is not busy anymore
    while (I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY));
    
    // Generate start condition
    I2C_GenerateSTART(I2Cx, ENABLE);
    
    // Wait for I2C EV5. 
    // It means that the start condition has been correctly released 
    // on the I2C bus (the bus is free, no other devices is communicating))
    while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT));
}

void i2c_stop()
{
    // Generate I2C stop condition
    I2C_GenerateSTOP(I2Cx, ENABLE);
    // Wait until I2C stop condition is finished
    while (I2C_GetFlagStatus(I2Cx, I2C_FLAG_STOPF));
}

void i2c_address_direction(uint8_t address, uint8_t direction)
{
    // Send slave address
    I2C_Send7bitAddress(I2Cx, address, direction);
    
    // Wait for I2C EV6
    // It means that a slave acknowledges his address
    if (direction == I2C_Direction_Transmitter)
    {
        while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
    }
    else if (direction == I2C_Direction_Receiver)
    {   
        while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
    }
}

void i2c_transmit(uint8_t byte)
{
    // Send data byte
    I2C_SendData(I2Cx, byte);
    // Wait for I2C EV8_2.
    // It means that the data has been physically shifted out and 
    // output on the bus)
    while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
}

uint8_t i2c_receive_ack()
{
    // Enable ACK of received data
    I2C_AcknowledgeConfig(I2Cx, ENABLE);
    // Wait for I2C EV7
    // It means that the data has been received in I2C data register
    while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED));
    
    // Read and return data byte from I2C data register
    return I2C_ReceiveData(I2Cx);
}

uint8_t i2c_receive_nack()
{
    // Disable ACK of received data
    I2C_AcknowledgeConfig(I2Cx, DISABLE);
    // Wait for I2C EV7
    // It means that the data has been received in I2C data register
    while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED));
    
    // Read and return data byte from I2C data register
    return I2C_ReceiveData(I2Cx);
}

void i2c_write(uint8_t address, uint8_t data)
{
    i2c_start();
    i2c_address_direction(address << 1, I2C_Direction_Transmitter);
    i2c_transmit(data);
    i2c_stop();
}

void i2c_read(uint8_t address, uint8_t* data)
{
    i2c_start();
    i2c_address_direction(address << 1, I2C_Direction_Receiver);
    *data = i2c_receive_nack();
    i2c_stop();
}

int uart_open(uint32_t baud, uint32_t flags)
{

    GPIO_InitTypeDef GPIO_InitStruct; 
    GPIO_StructInit(&GPIO_InitStruct);

    // Initialize GPIO TX
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2; 
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz; 
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP; 
    GPIO_Init(GPIOA, &GPIO_InitStruct);
    // Initialize GPIO RX
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3; 
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
    GPIO_Init(GPIOA, &GPIO_InitStruct);


    // see stm32f10x_usart.h
    USART_InitTypeDef USART_InitStructure;
    // Initialize USART structure
    USART_StructInit(&USART_InitStructure);
    USART_InitStructure.USART_BaudRate = baud; 
    USART_InitStructure.USART_Mode = flags;
    USART_Init(USART2,&USART_InitStructure);
    USART_Cmd(USART2, ENABLE);

}

int uart_putc(int c, USART_TypeDef* USARTx)
{
	// Aspetta che il data register sia vuoto per trasmettere
    while (USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET);
    // Mette nel data register il byte da trasmettere 
    USARTx->DR = (c & 0xff);
    return 0;
}

int uart_getc(USART_TypeDef* USARTx)
{
	// Controlla che il registro sia pieno prima di leggere
    while (USART_GetFlagStatus(USARTx, USART_FLAG_RXNE) == RESET); 
    // Si ritorna il dato del data register
    return USARTx->DR & 0xff;
}

void EXTI15_10_IRQHandler(void)
{ 
    if (EXTI_GetITStatus(EXTI_Line13) != RESET)
    {
        //i2c_write(SLAVE_ADDRESS, 0x01);
        //i2c_read(SLAVE_ADDRESS, &receivedByte);
        button = 1 - button ;
        EXTI_ClearITPendingBit(EXTI_Line13); 
    } 
}

static __IO uint32_t TimingDelay;

void Delay(uint32_t nTime){ 
    TimingDelay = nTime; 
    while(TimingDelay != 0);
}

void SysTick_Handler(void){ 
    if (TimingDelay != 0x00)
    TimingDelay--; 
}


#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line) {
/* Infinite loop */
/* Use GDB to find out why we're here */ while (1);
} 
#endif
