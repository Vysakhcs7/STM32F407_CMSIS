#include "stm32f407xx.h"

// Function prototypes
void GPIO_Init(void);
void Clock_Init(void);
void Timer2_Init(void);
void TIM2_IRQHandler(void);
void Timer4_Init(void);
void TIM4_IRQHandler(void);

// Global variables to track capture values and pulse width
static volatile uint32_t current_capture_value = 0;
static volatile uint32_t last_capture_value = 0;
static volatile uint32_t time_period = 0;
static volatile uint16_t pulse_CH1 = 0; // Pulse width for Channel 1 (PD12)

int main(void)
{
  // Initialize system clocks, GPIOs, and timers
  Clock_Init();
  GPIO_Init();
  Timer4_Init(); // Configure Timer 4 for PWM generation
  Timer2_Init(); // Configure Timer 2 for input capture

  // Main loop
  while(1)
  {
    // Main loop code here (if needed)
  }
}

// Function to initialize system clocks for GPIO and timers
void Clock_Init(void)
{
  // Enable clock access to GPIOA (for Timer 2 Channel 1)
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
  
  // Enable clock access to GPIOD (for Timer 4 Channel 1)
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
      
  // Enable clock access to Timer 2 (for input capture)
  RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
  
  // Enable clock access to Timer 4 (for PWM output)
  RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
}

// Function to initialize GPIO pins for Timer channels
void GPIO_Init(void)
{
  // Configure PD12 as Alternate Function mode (AF2 for TIM4_CH1)
  GPIOD->MODER &= ~GPIO_MODER_MODE12_0; // Clear mode bits for PD12
  GPIOD->MODER |= GPIO_MODER_MODE12_1; // Set PD12 to Alternate Function mode
  
  // Set PD12 to AF2 (Alternate Function 2) for TIM4 (AF2 selection)
  GPIOD->AFR[1] |= GPIO_AFRH_AFRH4_1;
  
  // Configure PA5 to Alternate Function mode (AF1 for TIM2_CH1)
  GPIOA->MODER |= GPIO_MODER_MODE5_1; // Set PA5 to Alternate Function mode
  GPIOA->MODER &= ~GPIO_MODER_MODE5_0; // Clear PA5 mode bits
  
  // Set PA5 to AF1 (AF1 selection for TIM2)
  GPIOA->AFR[0] |= GPIO_AFRL_AFSEL5_0;
}

// Function to initialize Timer 2 in input capture mode
void Timer2_Init(void)
{
  // Configure Timer 2 to capture input on Channel 1
  TIM2->CCMR1 |= TIM_CCMR1_CC1S_0; // Select input capture for Channel 1
  TIM2->CCMR1 &= ~TIM_CCMR1_CC1S_1; // Clear CC1S bits
  
  // Configure capture polarity and prescaler
  TIM2->CCER |= TIM_CCER_CC1P | TIM_CCER_CC1NP; // Capture on both edges
  TIM2->CCMR1 &= ~TIM_CCMR1_IC1PSC; // No prescaler
  
  // Enable capture and interrupt
  TIM2->CCER |= TIM_CCER_CC1E; // Enable capture on Channel 1
  TIM2->DIER |= TIM_DIER_CC1IE; // Enable capture interrupt
  
  // Enable Timer 2 interrupt in NVIC
  NVIC_EnableIRQ(TIM2_IRQn);
  
  // Start Timer 2
  TIM2->CR1 |= TIM_CR1_CEN;
}

// Timer 2 interrupt handler
void TIM2_IRQHandler(void)
{
  if (TIM2->SR & TIM_SR_CC1IF)
  {
    TIM2->SR &= ~TIM_SR_CC1IF; // Clear interrupt flag
    current_capture_value = TIM2->CCR1; // Read captured value
    time_period = current_capture_value - last_capture_value; // Compute time period
    last_capture_value = current_capture_value; // Update last captured value
  }
}

// Function to initialize Timer 4 for PWM output
void Timer4_Init(void)
{
  // Set Timer 4 prescaler to achieve 1 kHz timer clock with 16 MHz system clock
  TIM4->PSC = 1600 - 1;
  
  // Set Timer 4 auto-reload register for a 1-second interval at 1 kHz
  TIM4->ARR = 10000 - 1;
  
  // Configure Timer 4 Channel 1 (PD12) in toggle mode
  TIM4->CCMR1 |= TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_0; // Toggle mode
  TIM4->CCER |= TIM_CCER_CC1E; // Enable output on Channel 1
  TIM4->CCR1 = 0; // Initial compare value
  
  // Enable interrupts for Channel 1
  TIM4->DIER |= TIM_DIER_CC1IE;
  
  // Enable Timer 4 interrupt in NVIC
  NVIC_EnableIRQ(TIM4_IRQn);
  
  // Clear Timer 4 counter register
  TIM4->CNT = 0;
  
  // Start Timer 4
  TIM4->CR1 |= TIM_CR1_CEN;
}

// Timer 4 interrupt handler
void TIM4_IRQHandler(void)
{
  if (TIM4->SR & TIM_SR_CC1IF)
  {
    TIM4->SR &= ~TIM_SR_CC1IF; // Clear interrupt flag
  
    // Update pulse width for Channel 1
    pulse_CH1 = (pulse_CH1 + 100) % 1000; // Increment pulse width by 100 and wrap around at 1000
    TIM4->CCR1 = pulse_CH1; // Set new compare value for Channel 1
  }
}
