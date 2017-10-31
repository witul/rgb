
#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>

#define DEBOUNCE 2000

#define SW_COLOR_PRESSED !(PIND & (1<<PD3))
#define SW_DIM_PRESSED !(PIND & (1<<PD4))

#define DIM_STEPS 4
#define COLORS 7


volatile uint8_t f=0;

static uint8_t EEMEM initial_dim=2;
static uint8_t EEMEM initial_color=3;

static const uint8_t _brightnessAdj[] PROGMEM = {
    0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,2,
    2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,
    4,4,4,4,4,4,5,5,5,5,5,6,6,6,6,6,
    7,7,7,7,8,8,8,8,9,9,9,10,10,10,10,11,
    11,11,12,12,12,13,13,13,14,14,15,15,15,16,16,17,
    17,17,18,18,19,19,20,20,21,21,22,22,23,23,24,24,
    25,25,26,26,27,28,28,29,29,30,31,31,32,32,33,34,
    34,35,36,37,37,38,39,39,40,41,42,43,43,44,45,46,
    47,47,48,49,50,51,52,53,54,54,55,56,57,58,59,60,
    61,62,63,64,65,66,67,68,70,71,72,73,74,75,76,77,
    79,80,81,82,83,85,86,87,88,90,91,92,94,95,96,98,
    99,100,102,103,105,106,108,109,110,112,113,115,116,118,120,121,
    123,124,126,128,129,131,132,134,136,138,139,141,143,145,146,148,
    150,152,154,155,157,159,161,163,165,167,169,171,173,175,177,179,
    181,183,185,187,189,191,193,196,198,200,202,204,207,209,211,214,
    216,218,220,223,225,228,230,232,235,237,240,242,245,247,250,252
};

static const uint8_t _dimmingAdj[] PROGMEM = {25,100,175,255};

static const uint8_t colors[COLORS][3] PROGMEM = {
    {0,255,0},
    //    {0,0,255},
    {255,130,50},
    {255,255,153},
    {255,204,153},
    {204,255,204},
    {255,204,102},
    {255,0,0}
};



static uint8_t brightnessAdj(const uint8_t duty){
    return pgm_read_byte(_brightnessAdj+duty);
}

static uint8_t dimmingAdj(const uint8_t color, uint8_t step_index){

    uint8_t adj=(uint8_t) pgm_read_byte(_dimmingAdj+step_index);
    return color * adj / 255;
}

static uint8_t calculateColor(const uint8_t color,uint8_t step_index){
    return brightnessAdj(dimmingAdj(color,step_index));
}

static void set_dim(const PGM_VOID_P colorPtr,uint8_t step){

    //niebieski
    OCR0A =  calculateColor(pgm_read_byte(colorPtr+2),step);
    //zielony
    OCR1A =  calculateColor(pgm_read_byte(colorPtr+1),step);
    //czerwony
    OCR1B =  calculateColor(pgm_read_byte(colorPtr),step);
}

uint8_t dim=3;
uint8_t color=0;
const PGM_VOID_P colorsPtr;
PGM_VOID_P ptr;

void powerOn(){

}
void powerOff(){

}

static void updateLed(){
    set_dim(ptr,dim);
}

static void nextColor(){
    if(ptr==colorsPtr+3*(COLORS-1))
        ptr=colorsPtr;
    else ptr+=3;
}

static void nextDim(){
    if(dim == DIM_STEPS - 1)
        dim=0;
    else dim++;
}

void main(){

    DDRD = 0x00;
    PORTD = 0xFF;

    DDRB &=~((1<<PB0)|(1<<PB1));
    PORTB |=(1<<PB0)|(1<<PB1);

    DDRB |= (1<<PB2) | (1<<PB4) | (1<<PB3);
    PORTB |= (1<<PB2) | (1<<PB4) | (1<<PB3);

    TCCR0A |= (1<<WGM00) | (1<<COM0A1);
    TCCR0B |= (1<<CS00);

    TCCR1A |= (1<<WGM10) | (1<<COM1A1) | (1<<COM1B1);
    TCCR1B |= (1<<CS10);

    //eeprom_write_byte(&initial_dim,2);
    //eeprom_write_byte(&initial_color,3);

    TIMSK |= (1<<OCIE0B);
    //power button
    GIMSK |= (1<<INT0);
    //        |(1<<PCIE2);                 // Enable INT0
    MCUCR |= (1<<ISC00)|(1<<ISC01);    // Trigger INT0 on rising edge
    //PCMSK2|=(1<<PCINT15)|(1<<PCINT14);
    //GIFR |=(1<<INTF0);
    OCR0B = 250;
    sei();

    colorsPtr=pgm_get_far_address(colors[0]);
    ptr=colorsPtr;

    if(eeprom_is_ready()){
        dim=eeprom_read_byte(&initial_dim);
        color=eeprom_read_byte(&initial_color);
        ptr=colorsPtr+(3*color);
    }

    ptr=colorsPtr+(3*5);
    dim=3;
    updateLed();


    //uint8_t i=0,j=0;

    uint8_t s=0;
    while (1)
    {
        if(f!=s){
            if(f==1){
                updateLed();
            }
            else{
                OCR0A = 0;
                //zielony
                OCR1A =0;
                //czerwony
                OCR1B =0;
            }
            s=f;
        }
        //_delay_ms(100);
        /*
        for(i=0;i<COLORS;i++){
            dim=0;
            for(j=0;j<DIM_STEPS;j++){
                _delay_ms(1000);
                updateLed();
                nextDim();
            }
            nextColor();
        }*/
    }
}

ISR(TIMER0_COMPB_vect){
    static uint16_t t=0;
    t++;
    if(t>60000){
        f=f==0?1:0;
        t=0;
   }
}
