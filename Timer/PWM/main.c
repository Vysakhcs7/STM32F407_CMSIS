#include "stm32f4xx.h"  // Include the STM32F4xx peripheral library header file

#define PERIOD  500  // Define the PWM period
#define DUTY    250   // Define the PWM duty cycle

// Function prototypes
void GPIO_Init(void);     // Function to initialize GPIO settings
void Timer4_Init(void);   // Function to initialize Timer 4 settings
void TIM4_IRQHandler(void); // Function for Timer 4 interrupt handler (not used in this code)

// Main function
int main(void) {
    GPIO_Init();   // Initialize GPIOD for PD12
    Timer4_Init(); // Initialize Timer 4 for PWM generation

    while (1) {
        // Main loop is empty; Timer 4 handles PWM generation automatically
    }
}

// GPIO Initialization for PD12
void GPIO_Init(void) {
    // Enable clock for GPIOD
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;

    // Configure PD12 as an alternate function
    GPIOD->MODER &= ~GPIO_MODER_MODE12;        // Clear the mode bits for PD12
    GPIOD->MODER |= GPIO_MODER_MODE12_1;       // Set PD12 to Alternate Function mode
    
    // Set PD12 to AF2 (Alternate Function 2) for TIM4
    GPIOD->AFR[1] |= GPIO_AFRH_AFSEL12_1;
}

// Timer 4 Initialization
void Timer4_Init(void) {
    // Enable clock for Timer 4
    RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;

    // Configure Timer 4 in PWM mode 1
    TIM4->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1;  // Set PWM mode 1 (Active High) for Channel 1

    // Select Output Compare mode for Channel 1
    TIM4->CCMR1 &= ~TIM_CCMR1_CC1S;   // Clear CC1S bits to use output compare mode

    // Enable preload register for Channel 1
    TIM4->CCMR1 |= TIM_CCMR1_OC1PE;

    // Enable auto-reload preload
    TIM4->CR1 |= TIM_CR1_ARPE;

    // Generate an update event to reload the preloaded values
    TIM4->EGR |= TIM_EGR_UG;

    // Set output polarity to active high
    TIM4->CCER &= ~TIM_CCER_CC1P;

    // Set prescaler to divide timer clock by 16000
    TIM4->PSC = 16000 - 1;

    // Set the auto-reload register (ARR) for the PWM period
    TIM4->ARR = PERIOD;

    // Set the compare register (CCR1) for the PWM duty cycle
    TIM4->CCR1 = DUTY;

    // Enable the output compare function for Channel 1
    TIM4->CCER |= TIM_CCER_CC1E;

    // Start Timer 4
    TIM4->CR1 |= TIM_CR1_CEN;
}