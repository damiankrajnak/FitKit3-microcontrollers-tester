//       An example for demonstrating basic principles of FITkit3 usage.
//
// It includes GPIO - inputs from button press/release, outputs for LED control,
// timer in output compare mode for generating periodic events (via interrupt 
// service routine) and speaker handling (via alternating log. 0/1 through
// GPIO output on a reasonable frequency). Using this as a basis for IMP projects
// as well as for testing basic FITkit3 operation is strongly recommended.
//
//            (c) 2019 Michal Bidlo, BUT FIT, bidlom@fit.vutbr.cz
//
//            (c) EDITED by Damian Krajnak (xkrajn03), VUT FIT, xkrajn03@stud.fit.vutbr.cz
//
/* Header file with all the essential definitions for a given type of MCU */
#include "MK60D10.h"

// switch states
#define INPUT_OUTPUT_TEST 0
#define REGISTER_TEST 1

/* Macros for bit-level registers manipulation */
#define GPIO_PIN_MASK 0x1Fu
#define GPIO_PIN(x) (((1)<<(x & GPIO_PIN_MASK)))

/* Mapping of LEDs and buttons to specific port pins: */
// Note: only D9, SW3 and SW5 are used in this sample app
#define LED_D9  0x20      // Port B, bit 5
#define LED_D10 0x10      // Port B, bit 4
#define LED_D11 0x8       // Port B, bit 3
#define LED_D12 0x4       // Port B, bit 2

#define BTN_SW2 0x400     // Port E, bit 10
#define BTN_SW3 0x1000    // Port E, bit 12
#define BTN_SW4 0x8000000 // Port E, bit 27
#define BTN_SW5 0x4000000 // Port E, bit 26
#define BTN_SW6 0x800     // Port E, bit 11

#define SPK 0x10          // Speaker is on PTA4

// pressed buttons flags
int pressed_sw5 = 0, pressed_sw3 = 0, pressed_sw2 = 0, pressed_sw4 = 0, pressed_sw6 = 0;
int beep_flag = 0;
int state = INPUT_OUTPUT_TEST;
unsigned int compare = 0x200;

// functions prototypes
void delay(long long bound);
void MCUInit(void);
void PortsInit(void);
void LPTMR0_IRQHandler(void);
void LPTMR0Init(int count);

// functions that makes diodes blink, followed by beep sound
void d9_blink(void);
void d10_blink(void);
void d11_blink(void);
void d12_blink(void);

// assembly functions that check for stuck-at fault in registers
void register_check(char *regToBeChecked);

int main(void)
{
    // Initialization
    MCUInit();
    PortsInit();
    LPTMR0Init(compare);

    // endless loop of the application
    while (1) {
        // SW6 button pressed => change the state
        if(!pressed_sw6 && !(GPIOE_PDIR & BTN_SW6)){
            pressed_sw6 = 1;
            // switching the states (each state tests specific microcontroller)
            state = (state + 1) % 2;
        }
        else if(GPIOE_PDIR & BTN_SW6){
            pressed_sw6 = 0;
        }
        switch(state) {
            // registers check - beeps when some bit of the register is stuck at 0 or 1
            case REGISTER_TEST:
                register_check("R0");     /* R0 check */
                register_check("R1");     /* R1 check */
                register_check("R2");     /* R2 check */
                register_check("R3");     /* R3 check */
                register_check("R4");     /* R4 check */
                register_check("R5");     /* R5 check */
                register_check("R6");     /* R6 check */
                register_check("R7");     /* R7 check */
                register_check("R8");     /* R8 check */
                register_check("R9");     /* R9 check */
                register_check("R10");    /* R10 check */
                register_check("R11");    /* R11 check */
                register_check("R12");    /* R12 check */
                register_check("SP");     /* SP check */
                register_check("LR");     /* LR check */
            break;
                
            // this test tests diodes D9 - D12 and also tests buttons SW2 - SW5
            case INPUT_OUTPUT_TEST:
                
                // pressing the SW5 button tests the D9 diode, which will blink - followed by beep sound
                if (!pressed_sw5 && !(GPIOE_PDIR & BTN_SW5)){
                    d9_blink();
                }
                else if (GPIOE_PDIR & BTN_SW5){
                    pressed_sw5 = 0;                      // sets the pressed flag to 0
                }
                
                
                
                // pressing the SW3 button tests the D10 diode, which will blink - followed by beep sound
                if (!pressed_sw3 && !(GPIOE_PDIR & BTN_SW3)){
                    d10_blink();
                }
                else if (GPIOE_PDIR & BTN_SW3){
                    pressed_sw3 = 0;                    // sets the pressed flag to 0
                }

                
                
                // pressing the SW2 button tests the D11 diode, which will blink - followed by beep sound
                if (!pressed_sw2 && !(GPIOE_PDIR & BTN_SW2)){
                    d11_blink();
                }
                else if (GPIOE_PDIR & BTN_SW5){
                    pressed_sw2 = 0;                     // sets the pressed flag to 0
                }
                
                
                
                // pressing the SW4 button tests the D12 diode, which will blink - followed by beep sound
                if (!pressed_sw4 && !(GPIOE_PDIR & BTN_SW4)){
                    d12_blink();
                }
                else if (GPIOE_PDIR & BTN_SW4){
                    pressed_sw4 = 0;                     // sets the pressed flag to 0
                }
            break;
            
            default:
            break;
        }
    }

    return 0;
}

/* A delay function */
void delay(long long bound){
  long long i;
  for(i=0;i<bound;i++);
}

/* Initialize the MCU - basic clock settings, turning the watchdog off */
void MCUInit(void){
    MCG_C4 |= ( MCG_C4_DMX32_MASK | MCG_C4_DRST_DRS(0x01) );
    SIM_CLKDIV1 |= SIM_CLKDIV1_OUTDIV1(0x00);
    WDOG_STCTRLH &= ~WDOG_STCTRLH_WDOGEN_MASK;
}

void PortsInit(void){
    /* Turn on all port clocks */
    SIM->SCGC5 = SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTE_MASK | SIM_SCGC5_PORTA_MASK;

    /* Set corresponding PTB pins (connected to LED's) for GPIO functionality */
    PORTB->PCR[5] = PORT_PCR_MUX(0x01); // D9
    PORTB->PCR[4] = PORT_PCR_MUX(0x01); // D10
    PORTB->PCR[3] = PORT_PCR_MUX(0x01); // D11
    PORTB->PCR[2] = PORT_PCR_MUX(0x01); // D12

    PORTE->PCR[10] = PORT_PCR_MUX(0x01); // SW2
    PORTE->PCR[12] = PORT_PCR_MUX(0x01); // SW3
    PORTE->PCR[27] = PORT_PCR_MUX(0x01); // SW4
    PORTE->PCR[26] = PORT_PCR_MUX(0x01); // SW5
    PORTE->PCR[11] = PORT_PCR_MUX(0x01); // SW6

    PORTA->PCR[4] = PORT_PCR_MUX(0x01);  // Speaker

    /* Change corresponding PTB port pins as outputs */
    PTB->PDDR = GPIO_PDDR_PDD(0x3C);     // LED ports as outputs
    PTA->PDDR = GPIO_PDDR_PDD(SPK);     // Speaker as output
    PTB->PDOR |= GPIO_PDOR_PDO(0x3C);    // turn all LEDs OFF
    PTA->PDOR &= GPIO_PDOR_PDO(~SPK);   // Speaker off, beep_flag is false
}

void LPTMR0_IRQHandler(void){
    // Set new compare value set by up/down buttons
    LPTMR0_CMR = compare;                // !! the CMR reg. may only be changed while TCF == 1
    LPTMR0_CSR |=  LPTMR_CSR_TCF_MASK;   // writing 1 to TCF tclear the flag
    beep_flag = !beep_flag;              // see beep_flag test in main()
}

void LPTMR0Init(int count){
    SIM_SCGC5 |= SIM_SCGC5_LPTIMER_MASK; // Enable clock to LPTMR
    LPTMR0_CSR &= ~LPTMR_CSR_TEN_MASK;   // Turn OFF LPTMR to perform setup
    LPTMR0_PSR = ( LPTMR_PSR_PRESCALE(0) // 0000 is div 2
                 | LPTMR_PSR_PBYP_MASK   // LPO feeds directly to LPT
                 | LPTMR_PSR_PCS(1)) ;   // use the choice of clock
    LPTMR0_CMR = count;                  // Set compare value
    LPTMR0_CSR =(  LPTMR_CSR_TCF_MASK    // Clear any pending interrupt (now)
                 | LPTMR_CSR_TIE_MASK    // LPT interrupt enabled
                );
    NVIC_EnableIRQ(LPTMR0_IRQn);         // enable interrupts from LPTMR0
    LPTMR0_CSR |= LPTMR_CSR_TEN_MASK;    // Turn ON LPTMR0 and start counting
}
    
void d9_blink(void){
    pressed_sw5=1;                       // sets the pressed flag to 1
    GPIOB_PDOR ^= LED_D9;                // invert D9 statefor
    for (int i = 0; i<100; i++){         // loop determining beeping length
        GPIOA_PDOR ^= SPK;               // invert speaker state
        delay(1000);                     // sets frequence
    }
    GPIOB_PDOR ^= LED_D9;                // invert D9 state
}
    
void d10_blink(void){
    pressed_sw3=1;                       // sets the pressed flag to 1
    GPIOB_PDOR ^= LED_D10;               // invert D10 state
    for (int i = 0; i<100; i++){         // loop determining beeping length
        GPIOA_PDOR ^= SPK;               // invert speaker state
        delay(1000);                     // sets frequence
    }
    GPIOB_PDOR ^= LED_D10;               // invert D10 state
}
    
void d11_blink(void){
    pressed_sw2=1;                       // sets the pressed flag to 1
    GPIOB_PDOR ^= LED_D11;               // invert D11 state
    for (int i = 0; i<100; i++){         // loop determining beeping length
        GPIOA_PDOR ^= SPK;               // invert speaker state
        delay(1000);                     // sets frequence
    }
    GPIOB_PDOR ^= LED_D11;               // invert D11 state
}
    
void d12_blink(void){
    pressed_sw4=1;                       // sets the pressed flag to 1
    GPIOB_PDOR ^= LED_D12;               // invert D12 state
    for (int i = 0; i<100; i++){         // loop determining beeping length
        GPIOA_PDOR ^= SPK;               // invert speaker state
        delay(1000);                     // sets frequence
    }
    GPIOB_PDOR ^= LED_D12;               // invert D12 state
}

void register_check(char *regToBeChecked){
    int aaa = 0xAAAAAAAA;
    int fives = 0x55555555;
    __asm ("BAL LABEL");
    __asm ("ERROR_LABEL:");
        for(unsigned long int i = 0; i < 4000000000; i++){
            if (beep_flag){
                GPIOA_PDOR ^= SPK;   // invert speaker state
                delay(1000);
            }
            else GPIOA_PDOR &= ~SPK; // logic 0 on speaker port if beep is false
        }
    __asm ("LABEL:");
    __asm ("LDR %[output_a], =0xAAAAAAAA"       // loading 0xAAAAAAAA value to the register to be checked
           : [output_a] "=r" (regToBeChecked)
           );
    __asm ("CMP %[input_b], 0xAAAAAAAA"         // comparing value of the register to be checked, with the value expected
           :
           : [input_b] "r" (regToBeChecked)
           );
    __asm ("BNE ERROR_LABEL");                  // if not equal, jump to the ERROR_LABEL

    __asm ("LDR %[output_c], =0x55555555"       // loading 0x55555555 value to the register to be checked
           : [output_c] "=r" (regToBeChecked)
           );
    __asm ("CMP %[input_d], 0x55555555"         // comparing value of the register to be checked, with the value expected
           :
           : [input_d] "r" (regToBeChecked)
           );
    __asm ("BNE ERROR_LABEL");                  // if not equal, jump to the ERROR_LABEL
}
