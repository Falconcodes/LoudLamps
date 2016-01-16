/*
 * loudmeter.c
 *
 * Created: 12.12.2015 20:57:14
 * Author: Falcon
 */

#include <mega328p.h>
#include <delay.h>
#include <stdio.h>

#define DDRLED   DDRB.5
#define DDRLAMPS  DDRD=255; DDRB.2

#define LED PORTB.5
#define L1  PORTD.3
#define L2  PORTD.5
#define L3  PORTD.6
#define L4  PORTD.4
#define L5  PORTD.2
#define L6  PORTD.7
#define L7  PORTB.2



// Voltage Reference: AVCC pin
#define ADC_VREF_TYPE ((0<<REFS1) | (1<<REFS0) | (0<<ADLAR))

int i;
unsigned long int adc, silent_time;
float result, silent, loud, max_loud;

// Read the AD conversion result
unsigned int read_adc(unsigned char adc_input)
{
ADMUX=adc_input | ADC_VREF_TYPE;
// Delay needed for the stabilization of the ADC input voltage
delay_us(10);
// Start the AD conversion
ADCSRA|=(1<<ADSC);
// Wait for the AD conversion to complete
while ((ADCSRA & (1<<ADIF))==0);
ADCSRA|=(1<<ADIF);
return ADCW;
}

void main(void)
{
DDRLED=1;
DDRLAMPS=1;
L1=L2=L3=L4=L5=L6=L7=0;

// ADC Clock: 1000,000 kHz Reference: AVCC pin Trigger Source: Free Running ADC0: On, ADC1: Off, ADC2: Off, ADC3: Off ADC4: Off, ADC5: Off
DIDR0=(1<<ADC5D) | (1<<ADC4D) | (1<<ADC3D) | (1<<ADC2D) | (1<<ADC1D) | (0<<ADC0D);
ADMUX=ADC_VREF_TYPE;
ADCSRA=(1<<ADEN) | (0<<ADSC) | (1<<ADATE) | (0<<ADIF) | (0<<ADIE) | (1<<ADPS2) | (0<<ADPS1) | (0<<ADPS0);
ADCSRB=(0<<ADTS2) | (0<<ADTS1) | (0<<ADTS0);

/*
// Communication Parameters: 8 Data, 1 Stop, No Parity Receiver: Off Transmitter: On Mode: Asynchronous Baud Rate: 9600
UCSR0A=(0<<RXC0) | (0<<TXC0) | (0<<UDRE0) | (0<<FE0) | (0<<DOR0) | (0<<UPE0) | (0<<U2X0) | (0<<MPCM0);
UCSR0B=(0<<RXCIE0) | (0<<TXCIE0) | (0<<UDRIE0) | (0<<RXEN0) | (1<<TXEN0) | (0<<UCSZ02) | (0<<RXB80) | (0<<TXB80);
UCSR0C=(0<<UMSEL01) | (0<<UMSEL00) | (0<<UPM01) | (0<<UPM00) | (0<<USBS0) | (1<<UCSZ01) | (1<<UCSZ00) | (0<<UCPOL0);
UBRR0H=0x00;
UBRR0L=0x67;
*/

 //калибровка при перезапуске контроллера - определяется уровень "тишины". Надо обеспечить тишину при включении/перезагрузке.
 LED=1; //индикация начала калибровки - зажигаем светодиод
 L1=L2=L3=L4=L5=L6=L7=1; // и все лампы
 
 for (i=0; i<10000; i++) {
   adc = read_adc (7); //read ADC7 input channel
   result = ((float)adc + 1) * 4.7 / 1024; 
   silent+=result;
 }
silent/=10000.0; //получаем среднее значение ADC при тишине
LED=0; //калибровкка окончена - гасим светодиод на плате
L1=L2=L3=L4=L5=L6=L7=0; //и все лампы

while(1){
 
 if ((loud < 100) && (max_loud > 200)) {   //считаем время тишины, если перед этим кричали (если замечен уровень звука > 200)
 silent_time++;
  if (silent_time == 30) {   //если несколько секунд не было звуков, значит перестали кричать => показываем результат
   silent_time = 0;
   loud = max_loud; //выводим максимальный результат
   
   for (i=0; i<30; i++) {
   if (loud>50)   L1=1;
   if (loud>200)  L2=1;
   if (loud>500)  L3=1;
   if (loud>700)  L4=1;
   if (loud>1000) L5=1;
   if (loud>1300) L6=1;
   if (loud>1500) L7=1;

   delay_ms(200);
   L1=L2=L3=L4=L5=L6=L7=0;
   delay_ms(30);
   }
   loud=0;
   max_loud=0;  //обнуляем итоги
  } 
 }  
 
 loud = 0;
  
 //определяем уровень шума, суммируя импульсы, большие чем (уровень тишины + 0.1) и меньшие чем (уровень тишины - 0.1)
  for (i=0; i<1000; i++) {
   adc = read_adc (7); //read ADC7 input channel
   result = ((float)adc + 1) * 4.7 / 1024; //пересчет в Вольты
   if ( result >= (silent + 0.1) ) loud += result - silent; //собираем импульсы выше среднего
   if ( result <= (silent - 0.1) ) loud += silent - result; //собираем импульсы ниже среднего
  }
 
 if (loud > max_loud) max_loud = loud; //обновляем максимум за исследуемый период времени
 
 if (loud > 100) silent_time = 0; //обнуляем счетчик времени тишины, если снова кричат
 
 L1=L2=L3=L4=L5=L6=L7=0;
   
 if (loud>50)   L1=1;
 if (loud>200)  L2=1;
 if (loud>500)  L3=1;
 if (loud>700)  L4=1;
 if (loud>1000) L5=1;
 if (loud>1300) L6=1;
 if (loud>1500) L7=1;
 }
}
