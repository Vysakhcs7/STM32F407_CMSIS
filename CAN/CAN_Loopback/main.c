/*
Enable Clock:
//PB8 - CAN1_Rx
//PB9 - CAN1_Tx
// 1001: AF9
Steps to Configure CAN in Loopback Mode
Enable the clock for the CAN peripheral in the RCC (Reset and Clock Control) registers.
Set up the appropriate GPIO pins for CAN communication.Ensure the pins are set to alternate function mode,
with the correct settings for speed, output type, pull-up/pull-down resistors, etc.
Configure the CAN peripheral setting parameters such as:
Bit Timing: Configure the prescaler, time segments, and synchronization jump width to define the CAN bus speed (e.g., 125 kbps, 500 kbps).
Mode: Set the CAN mode to loopback mode, which allows the CAN controller to send and receive its own messages.
Configure the CAN filters to accept all messages. In loopback mode, you can set the filter to a basic setting to 
ensure any transmitted messages are received.
Typically, you would set the filter to allow all IDs (standard and extended) by configuring the filter mask and filter list.
Start the CAN Peripheral:
Activate the CAN peripheral by enabling it in the control register.
Prepare a CAN message structure and load it with the desired data and identifier.
Use the appropriate function to transmit the message through the CAN peripheral.
Receive a Message:
In loopback mode, any message transmitted by the CAN peripheral should be received by the same peripheral. Check the receive FIFO or message object for incoming messages.
Verify Communication:
Check if the transmitted message is successfully received. This can typically be done by reading the received message from the CAN receive FIFO and comparing it with the transmitted message.
*/

#include "stm32f407xx.h"
void GPIO_Init(void);
void CANBus_Init(void);
void CAN1_LoopBack_Mode(void);
void Delay(void);
static void CAN1_Tx(void);
void CANBus_Start(void);


void GPIO_Init(void){
  //PORTB Clock Enable
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
  //Alternate Function Mode
  GPIOB->MODER|=GPIO_MODER_MODE8_1|GPIO_MODER_MODE9_1;
	GPIOB->MODER&=~(GPIO_MODER_MODE8_0|GPIO_MODER_MODE9_0);
  //Set AF9
  GPIOB->AFR[1] |= GPIO_AFRH_AFSEL9_0 | GPIO_AFRH_AFSEL9_3;
  //Pullup
  GPIOB->PUPDR |= GPIO_PUPDR_PUPD8_0 | GPIO_PUPDR_PUPD9_0; // Enable pull-up for PB8 and PB9  
  //Speed
  GPIOB->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR8 | GPIO_OSPEEDER_OSPEEDR9; // High speed for PB8 and PB9

}

void CANBus_Init(void){
  //CAN1 Clock Enable
  RCC->APB1ENR |= RCC_APB1ENR_CAN1EN;
      /* Enter Initialization mode*/
    CAN1->MCR |= CAN_MCR_INRQ;
    /*Wait until CANBus peripheral is in initialization mode*/
    //while (!(CAN1->MSR & CAN_MSR_INAK)){}
      while ((CAN1->MSR & CAN_MSR_INAK)==0U);
   // Enable CAN1 peripheral
    CAN1->MCR &= ~CAN_MCR_SLEEP;
    while ((CAN1->MSR & CAN_MSR_SLAK)!=0U);
  
  //Loop back mode enable
      CAN1->BTR |= CAN_BTR_LBKM; 
    //Baud rate prescalar
  CAN1->BTR |= 1 << 1; //Prescalar = 2
  //Maximum time quanta
  CAN1->BTR |= (15 << 24); //Time quanta set to 16
    CAN1->BTR |= (12 << 16);    // Set TS1 to 13
    CAN1->BTR |= (1 << 20);    // Set TS2 to 2
    //No Filter used
    CAN1->FA1R = 0;
    	Filter_Configuration();

}

void CANBus_Start(void)
{
    // Leave Initialization mode
    CAN1->MCR &= ~CAN_MCR_INRQ;
    while (CAN1->MSR & CAN_MSR_INAK);

}

void CAN1_Tx(void){
 CAN1->sTxMailBox[0].TIR |= CAN_TI0R_TXRQ;
 while(!(CAN1->TSR & CAN_TSR_TXOK0));
 while(!(CAN1->TSR & CAN_TSR_RQCP0));
 CAN1->TSR |= 1 << CAN_TSR_RQCP0;
   
}

void Delay(void)
{
  for(volatile int i = 0; i < 70000; i++);
}

int main(void)
{
  GPIO_Init();
  CANBus_Init();
  CANBus_Start();
  while(1)
  {
    CAN1_Tx();
    Delay();
  }
}



