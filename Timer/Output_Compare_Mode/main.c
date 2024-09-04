#include "stm32f407xx.h"

// Function prototypes
void GPIO_Init(void);
void Clock_Init(void);
void Timer4_Init(void);
void TIM4_IRQHandler(void);

// Global variables to store pulse values for Timer Channels
volatile uint16_t pulse_CH1 = 0;  // Pulse width for Channel 1 (PD12)
volatile uint16_t pulse_CH2 = 0;  // Pulse width for Channel 2 (PD13)

int main(void)
{
  Clock_Init();  // Initialize the system clocks
  GPIO_Init();   // Configure GPIO pins
  Timer4_Init(); // Configure Timer 4 for PWM generation
  
  while(1)
  {
    // Main loop is empty; all functionality is handled in the timer interrupt
  }  
}

void Clock_Init(void)
{ 
  // Enable clock access to GPIOD peripheral
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
      
  // Enable clock access to Timer 4 peripheral
  RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
}

void GPIO_Init(void)
{
  // Configure PD12 and PD13 as Alternate Function mode (AF2 for TIM4_CH1 and TIM4_CH2)
  GPIOD->MODER &= ~(GPIO_MODER_MODE12 | GPIO_MODER_MODE13); // Clear mode bits for PD12 and PD13
  GPIOD->MODER |= (GPIO_MODER_MODE12_1 | GPIO_MODER_MODE13_1); // Set PD12 and PD13 to Alternate Function mode
  
  // Set PD12 and PD13 to AF2 (Alternate Function 2) for TIM4
  GPIOD->AFR[1] &= ~(GPIO_AFRH_AFSEL12 | GPIO_AFRH_AFSEL13); // Clear AF bits for PD12 and PD13
  GPIOD->AFR[1] |= GPIO_AFRH_AFSEL12_1 | GPIO_AFRH_AFSEL13_1; // Set AF2 (Alternate Function 2) for PD12 and PD13
}

void Timer4_Init(void)
{
  // Set Timer 4 prescaler to 15999 to achieve 1 kHz timer clock with a 16 MHz system clock
  TIM4->PSC = 16000 - 1;
  
  // Set Timer 4 auto-reload register to 999 for a 1-second interval at 1 kHz
  TIM4->ARR = 1000 - 1;

  // Configure Channel 1 (PD12) to toggle mode
  TIM4->CCMR1 |= TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_0; // Set toggle mode for Channel 1
  TIM4->CCER |= TIM_CCER_CC1E; // Enable output for Channel 1
  TIM4->CCR1 = 0; // Initial compare value for Channel 1

  // Configure Channel 2 (PD13) to toggle mode
  TIM4->CCMR1 |= TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_0; // Set toggle mode for Channel 2
  TIM4->CCER |= TIM_CCER_CC2E; // Enable output for Channel 2
  TIM4->CCR2 = 0; // Initial compare value for Channel 2
  
  // Enable interrupts for both channels
  TIM4->DIER |= TIM_DIER_CC1IE | TIM_DIER_CC2IE;

  // Clear Timer 4 counter register
  TIM4->CNT = 0;
  
  // Start Timer 4 by enabling the counter
  TIM4->CR1 |= TIM_CR1_CEN;

  // Enable Timer 4 interrupt in NVIC
  NVIC_EnableIRQ(TIM4_IRQn);
}

void TIM4_IRQHandler(void)
{
  // Check if interrupt is due to Channel 1 compare match
  if (TIM4->SR & TIM_SR_CC1IF)
  {
    // Clear the Channel 1 compare match interrupt flag
    TIM4->SR &= ~TIM_SR_CC1IF;
    
    // Update the CCR1 register with a new pulse width value for Channel 1
    // Increment pulse width by 100 and wrap around at 1000
	pulse_CH1 = (pulse_CH1 + 1000) % 1000; 
    
    // Set new compare value for Channel 1 
    TIM4->CCR1 = pulse_CH1;  
  }
  
  // Check if interrupt is due to Channel 2 compare match
  if (TIM4->SR & TIM_SR_CC2IF)
  {
    // Clear the Channel 2 compare match interrupt flag
    TIM4->SR &= ~TIM_SR_CC2IF;
    
    // Update the CCR2 register with a new pulse width value for Channel 2
   // Increment pulse width by 50 and wrap around at 1000    
   pulse_CH2 = (pulse_CH2 + 100) % 1000;  

   // Set new compare value for Channel 2
   TIM4->CCR2 = pulse_CH2;  
  }
}