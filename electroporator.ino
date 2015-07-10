#include <LiquidCrystal.h>

#define NUM_PULSES   3
#define PULSE_DELAY  2
#define PULSE_WIDTH  1
#define ADC_VOLTS    0

#define OUT_LED      10 
#define OUT_SHOCK    9

#define LCD_D4       0
#define LCD_D5       1
#define LCD_D6       2
#define LCD_D7       3
#define LCD_RS       4
#define LCD_RW       5
#define LCD_EN       6

#define PEDAL        7
#define BUTTON       8

#define VOLTAGE_SCALER 100UL
#define WIDTH_SCALER   1000UL
#define DELAY_SCALER   5000UL
#define NUM_SCALER     20UL

#define CLOCK_SCALER   4

LiquidCrystal lcd(LCD_RS, LCD_RW, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

unsigned long voltage; 
unsigned long pulse_width; 
unsigned long pulse_delay; 
unsigned long num_pulses;
volatile int  trigger=0;
unsigned int  locked=0; 

void pulseOut(int ms)
{
  /* Set Signals High */
  //digitalWrite(OUT_LED, HIGH);
  digitalWrite(OUT_SHOCK, HIGH);

  /* Wait for some period of time */
  delay(ms);

  /* Set Signals Low */
  digitalWrite(OUT_SHOCK, LOW);
  //digitalWrite(OUT_LED,LOW); 
}

void setup()
{ 
  // Configure Pins 13 and 14 as Inputs
  pinMode(PEDAL, INPUT);
  // Enable Pullup to High
  digitalWrite(PEDAL, HIGH);
  
  // Setup LCD colums x rows
  lcd.begin(16,2);
  
  lcd.setCursor(0,0);
  lcd.print("UCLA Physics");
  lcd.setCursor(0,1);
  lcd.print("Electronics Shop"); 
  delay (200);

  lcd.setCursor(0,0);
  lcd.print("ucla.eshop      ");
  lcd.setCursor(0,1);
  lcd.print("@gmail.com      "); 
  delay (300);
  
  // Configure Pins 13 and 14 to respond to Pin Change Interrupts
  // See http://www.me.ucsb.edu/~me170c/Code/How_to_Enable_Interrupts_on_ANY_pin.pdf
  PCICR  |= (1<<PCIE2); // for pedal
  PCMSK2 |= (1<<PCINT23); // for pedal
  MCUCR   = (1<<ISC01);  // Trigger on Falling Edge
  MCUCR  &=~(1<<ISC00);  // Trigger on Falling Edge


  //turn on interrupts
  interrupts();

  //Configure and initialize outputs
  pinMode(OUT_SHOCK,OUTPUT);
  //pinMode(OUT_LED,OUTPUT);

  digitalWrite(OUT_SHOCK,LOW); 
  //digitalWrite(OUT_LED,LOW);
}

void loop() 
{
  /* Read ADCs and multiply by a scaler to give physical measurements 
   * each Analog read takes ~ 100 us */
  voltage     = (analogRead(ADC_VOLTS)   * VOLTAGE_SCALER) / 1023;
  pulse_width = (analogRead(PULSE_WIDTH) * WIDTH_SCALER)   / 1023;
  pulse_delay = (analogRead(PULSE_DELAY) * DELAY_SCALER)   / 1023;
  num_pulses  = (analogRead(NUM_PULSES)  * NUM_SCALER)     / 1023;

  /* Minimum 10 ms pulse delay */
  pulse_delay = max(10,pulse_delay); 
  pulse_width = max(1, pulse_width); 
  num_pulses  = max(1, num_pulses);

  /* Print something of the form: 
   * CV:100 PW:1000ms
   * Dly:1000ms #P:09 */
  char line1[16];
  char line2[16];
  sprintf(line1,"CV:%3lu PW:%4lums", voltage, pulse_width);
  sprintf(line2,"Dly:%4lums #P:%2lu",  pulse_delay, num_pulses);

  lcd.setCursor(0,0);
  lcd.print(line1);
  lcd.setCursor(0,1);
  lcd.print(line2); 


  if ((!locked) && (trigger) && (millis() > 500))
  {     
    locked  = 1; 
    for (uint8_t i=0; i<num_pulses; i++)
    {
      char text [2];
      sprintf(text,"%2i",i+1);
      lcd.setCursor(14,1);
      lcd.print(text);
      pulseOut(pulse_width/CLOCK_SCALER);
      if (i<(num_pulses-1))
        delay(pulse_delay/CLOCK_SCALER);
    } 
    delay(125); 
    locked=0;
    trigger = 0;
  }
}

/* Pin 7 Pedal Interrupt Sequence */
ISR (PCINT2_vect)
{
  static unsigned long last_interrupt_time = 0; 
  unsigned long interrupt_time = millis(); 
  if (interrupt_time - last_interrupt_time > 500)
  {
    trigger = 1;
  }
}

/* Pin 14 Button Interrupt Sequence (Currently Unused) */
//ISR (PCINT8_void)
//{
//  
//}

