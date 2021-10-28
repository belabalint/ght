#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <wiringPi.h>
#include <wiringShift.h>
#include <wiringPiI2C.h>
#include <wiringPiSPI.h>
#include <stdint.h>
#include <time.h>
#include "sinetable.h"
// change this to 2 for quick testing, keep it 18 otherwise
#define ELECTRODES 18
 
// time constants in seconds
#define TRANSIENTS_WAIT             5
#define SYNC_WAIT                   0.5
#define SINE_TIME                   3
#define ECG_WAIT                    30
#define DRL_WAIT                    30

 
// pins
#define TI_595_SRCK         10
#define TI_595_RCK          14
#define TI_595_SER_OUT      13
#define TI_595_SER_IN       12
#define AD7994_CONVST       5
 
#define TI_595_DDS_FSELECT  0
#define TI_595_DDS_RESET    1
#define TI_595_DDS_SDATA    2
#define TI_595_DDS_FSYNC    3
#define TI_595_DDS_SCLK     5
#define TI_595_DDS_SLEEP    6
#define TI_595_DDS_PSELECT  7
 
#define TI_595_AC_DAC_CLK   4
#define TI_595_AC_DAC_SRI   8
#define TI_595_AC_DAC_LD    15

 
// sync pins
#define PI_IR_REC           0
#define PI_PHTRIG           1
 
// Global variables
// this variable holds the status of the 595 chips
unsigned char TI_595_state_vector[80]; 
 
// file handles for I2C devices
int file_handle_ADC, file_handle_DAC, file_handle_DigPot; 
 
// this should not be a global variable
int error_code; 
 
// used to iterate through the electrode relays
// this variable is modified by increment_relay_address(char skip_cz)
// and also modified by reset_relay_address()
int relay_address;
 
// determines if CZ relay is to be skipped or not
typedef enum
{
    INCLUDING_CZ,
    EXCLUDING_CZ
} skip_cz_t;


void increment_relay_address(skip_cz_t skip_cz)
{
    // increment relay_address by 2
    relay_address = relay_address + 2;
    
    // skip CZ if neccesary
    if (relay_address == 35 && skip_cz == EXCLUDING_CZ)
    {
        relay_address = relay_address + 2;
    }
    
    // skip to T5
    if (relay_address == 57)
    {
        relay_address = 75;
    }   
}


// resets electrode address to FP1
void reset_relay_address()
{
    relay_address = 23;
}

	
void init_i2c_devices()
{
    printf("Initializing digital potentiometer...");
    file_handle_DigPot = wiringPiI2CSetup (0x18);
    if (file_handle_DigPot == -1) 
    {
        printf(" - UNREACHABLE\n");
    }
    else
    {
        printf(" - OK\n");
    }
    
    printf("Initializing AD5696R DAC...");
    file_handle_DAC = wiringPiI2CSetup (0x0C);
    if (file_handle_DAC == -1) 
    {
        printf(" - UNREACHABLE\n"); 
    }
    else
    {
        printf(" - OK\n");
    }
    
    printf("Initializing AD7994-1 ADC...");
    file_handle_ADC = wiringPiI2CSetup (0x23);
    if (file_handle_ADC == -1) 
    {
        printf(" - UNREACHABLE\n"); 
    }
    else
    {
        printf(" - OK\n");
    }
}
 
void update_595_state(void)
{
    unsigned int i;
    digitalWrite(TI_595_SER_OUT, LOW);
    digitalWrite(TI_595_SRCK, LOW);
    
    for (i=0; i<80; i++)
    {
        if (TI_595_state_vector[79-i] == '0') 
        {
            digitalWrite(TI_595_SER_OUT, LOW);
        }
        else 
        {
            digitalWrite(TI_595_SER_OUT, HIGH);
        }
        delayMicroseconds(1);
        digitalWrite(TI_595_SRCK, HIGH);
        delayMicroseconds(1);
        digitalWrite(TI_595_SRCK, LOW);
    }
    digitalWrite(TI_595_SER_OUT, LOW);
    delayMicroseconds(1);
    digitalWrite(TI_595_RCK, HIGH);
    delayMicroseconds(1);
    digitalWrite(TI_595_RCK, LOW);
    delayMicroseconds(1);   
}
// clocks out 16 bits of data
// NO FSYNC!
// used in DDS functions
void dds_send(uint16_t data)
{
    char bit;
    
    for(int i=15; i>=0 ; i--)
    {
        // put sdata out
        bit = (data >> i) & 0x0001;
        if (bit == 0)
        {
            TI_595_state_vector[TI_595_DDS_SDATA] = '1';
        }
        else 
        {
            TI_595_state_vector[TI_595_DDS_SDATA] = '0';
        }
        update_595_state();
        
        // clock sdata
        TI_595_state_vector[TI_595_DDS_SCLK] = '1';
        update_595_state();
        TI_595_state_vector[TI_595_DDS_SCLK] = '0';
        update_595_state(); 
    }   
}
 
// intilaizes DDS
void dds_init()
{
    printf("Initializing DDS...");
    
    // set 595 pins to initial values
    TI_595_state_vector[TI_595_DDS_FSYNC]   = '0';
    TI_595_state_vector[TI_595_DDS_SCLK]    = '0';
    TI_595_state_vector[TI_595_DDS_SDATA]   = '1';
    TI_595_state_vector[TI_595_DDS_FSELECT] = '1';
    TI_595_state_vector[TI_595_DDS_PSELECT] = '1';
    TI_595_state_vector[TI_595_DDS_SLEEP]   = '1';
    TI_595_state_vector[TI_595_DDS_RESET]   = '1';
    update_595_state();
    
    
    /* set the configuration register
    DB15 - 0
    DB14 - 0
    DB13 - 1: 28 bit mode enable
    DB12 - 0: half resolution mode, ingored since DB13 is 1
    DB11 - 0: FSEL - ignored: pin mode activated
    DB10 - 0: PSEL - ignored: pin mode activated
    DB09 - 1: FSEL, PSEL, SLEEP, RESET controlled by pins
    DB08 - 0: RESET, ignored
    DB07 - 0: SLEEP1, not reset, ignored
    DB06 - 0: SLEEP12, DAC enabled
    DB05 - 0: OPBITEN, sign bit not needed
    DB04 - 0: not needed
    DB03 - 0: not needed
    DB02 - 0: must be set to 0
    DB01 - 0: sign bit mode, not needed
    DB00 - 0: must be set to 0
    
    REGISTER VALUE: 0b0010001000000000
    */
    dds_send(0b0010001000000000);

    TI_595_state_vector[TI_595_DDS_RESET]   = '0';
    update_595_state();
    
    TI_595_state_vector[TI_595_DDS_RESET]   = '1';
    update_595_state();
        
    TI_595_state_vector[TI_595_DDS_FSYNC] = '1';
    update_595_state();
    
    printf(" - OK\n");
}
uint16_t swapper(uint16_t tmp)
{
    uint16_t dac_value;
    dac_value = tmp << 8;
    dac_value = dac_value | (tmp >> 8);
    return dac_value;
}
 
void dds_set_frequency(double f)
{
    const double fmck = 6000000;
    uint32_t freg = 0;
    uint16_t freg_lsb = 0;
    uint16_t freg_msb = 0;
    static char dds_fsel = 0; //why static??¿¿
    
    printf("Setting DDS frequency to %f Hz\n", f);
    
    freg = f*(pow(2, 28)/fmck);
    
    freg_lsb = freg & 0x3FFF;
    freg_msb = (freg >> 14) & 0x3FFF;
    
    // swaps between FTWs when changing frequency
    if (dds_fsel == 0)
    {
        dds_fsel = 1;
        freg_lsb = freg_lsb | 0x8000;
        freg_msb = freg_msb | 0x8000;
    }
    else
    {
        dds_fsel = 0;
        freg_lsb = freg_lsb | 0x4000;
        freg_msb = freg_msb | 0x4000;
    }
    
    TI_595_state_vector[TI_595_DDS_FSYNC] = '1';
    update_595_state();
    dds_send(freg_lsb);
    dds_send(freg_msb);
    TI_595_state_vector[TI_595_DDS_FSYNC] = '0';
    update_595_state();
        
    if (dds_fsel == 0)
    {
        TI_595_state_vector[TI_595_DDS_FSELECT] = '1';
    }
    else
    {
        TI_595_state_vector[TI_595_DDS_FSELECT] = '0';
    }
    update_595_state();
}
 
// m is for multiplication factor [0, 1]
void ac_dac_set(double m)
{
    char bit;
    uint16_t reg = 0;
    
    printf("Setting AC_DAC to %3.1f\%\n", m*100);
    
    // coerce m to [0, 1]
    if (m > 1) m = 1;
    if (m < 0) m = 0;
    
    reg = m*(pow(2, 12)-1);
    reg = reg << 4;
    reg = reg >> 4;
    TI_595_state_vector[TI_595_AC_DAC_LD] = '1';
    update_595_state();
    TI_595_state_vector[TI_595_AC_DAC_LD] = '0';
    update_595_state();
    
    for(int i=11; i>=0 ; i--)
    {
        TI_595_state_vector[TI_595_AC_DAC_CLK] = '1';
        update_595_state();
        
        // put sdata out
        bit = (reg >> i) & 0x0001;
        if (bit == 0)
        {
            TI_595_state_vector[TI_595_AC_DAC_SRI] = '1';
        }
        else 
        {
            TI_595_state_vector[TI_595_AC_DAC_SRI] = '0';
        }
        update_595_state();
        TI_595_state_vector[TI_595_AC_DAC_CLK] = '0';
        update_595_state(); 
    }
    
    TI_595_state_vector[TI_595_AC_DAC_LD] = '1';
    update_595_state();
    TI_595_state_vector[TI_595_AC_DAC_LD] = '0';
    update_595_state();
}
 
void ac_dac_init() // initializing digit. to analog
{
    printf("Initializing AC_DAC...");
 
    TI_595_state_vector[TI_595_AC_DAC_LD] = '0';
    TI_595_state_vector[TI_595_AC_DAC_CLK] = '0';
    TI_595_state_vector[TI_595_AC_DAC_SRI] = '1';
    update_595_state();
    ac_dac_set(0);
    
    printf(" - OK\n");
}


void system_init(void)
{
    printf("INIT START\n");
 
    printf("Initializing WiringPi...");
    wiringPiSetup();
    printf(" - OK\n");
    
    init_i2c_devices();
 
    printf("Initializing TI_595 pins...");
    // GPIO pins init (data direction)
    pinMode(TI_595_SRCK, OUTPUT);
    pinMode(TI_595_RCK, OUTPUT);
    pinMode(TI_595_SER_OUT, OUTPUT);
    pinMode(TI_595_SER_IN, INPUT);
    pinMode(AD7994_CONVST,OUTPUT);
    pinMode(PI_IR_REC, OUTPUT);
    pinMode(PI_PHTRIG , OUTPUT);
    
    // GPIO pins init value
    digitalWrite(TI_595_RCK, LOW);
    digitalWrite(TI_595_SRCK, LOW);
    digitalWrite(TI_595_SER_OUT, LOW);
    digitalWrite(AD7994_CONVST, LOW);
    digitalWrite(PI_IR_REC, LOW);
    digitalWrite(PI_PHTRIG, LOW);
    printf(" - OK\n");
    
    printf("Resetting 595 state vector...");
    // status of the 595 chips are set to all zero 
    for (int i = 0; i < 80; i++)
    {
        TI_595_state_vector[i]= '0'; // clear the 595 state 
    }
    update_595_state();
    printf(" - OK\n");
    
    // enable USB (drive usb_spst)
    TI_595_state_vector[57] = '1';
    update_595_state();
    
    printf("Powering USB...");
 
    dds_init();
    ac_dac_init();
    ac_dac_set(0);
    
    printf("INIT END\n");
}

 
void set_ad5696_dac_voltage(double voltage, int channel)
{
    uint16_t dac_value;
    uint16_t tmp;
    uint8_t command;
    int error_code;
    
    if (channel == 0) {
        command = 0b00110001;
    }
    else if (channel == 1) {
        command = 0b00110010;
    }
    else if (channel == 2) {
        command = 0b00110100;
    }
    else if (channel == 3) {
        command = 0b00111000;
    }
    else {
        command = 0b00110010;
    }
    printf("Setting ad_5696 voltage for channel %d to %fV...  ",channel, voltage);
    // setting the output voltage 
    tmp = round((voltage/5.0)*65535);
    dac_value = swapper(tmp);
    error_code = wiringPiI2CWriteReg16(file_handle_DAC, command, dac_value);
    
    printf("%i\n", error_code);
}

 
// wait for s seconds
void wait(double s)
{
    int t1, t2;
    if (s > 0)
    { 
        t1=millis();
        t2=millis();
        while ((t2-t1)/1000.0 <= s)
        {
            t2=millis();
            
            // this printf can mess up the terminal emulator 
            printf("Elapsed time: %2.2f s of %2.2f s\r", (t2-t1)/1000.0, s);
            delayMicroseconds(1000);
        }
        printf("\n");
    }
}

float convert_ad7994_output_to_volts(uint16_t outp) 
{
    outp = swapper(outp);
    outp = outp << 4;
    float tmp = outp/16;
    tmp = tmp/4095;
    tmp = tmp*5;
    return tmp;
    
}
/*uint16_t read_ad7994_voltage_alt()
{
    uint8_t configregisteraddress = 0b00000010;
    uint8_t configregisterdataon = 0b00011000;
    uint8_t configregisterdataoff = 0b00001000;
    uint8_t cycleregisteraddress = 0b00000011;
    uint8_t cycleregisterdataon = 0b00000001;
    uint8_t cycleregisterdataoff = 0b00000000;
    uint8_t conversionregisteraddress = 0;
    int error_code = wiringPiI2CWriteReg16(file_handle_ADC, configregisteraddress, configregisterdataon);
    error_code = wiringPiI2CWriteReg16(file_handle_ADC, cycleregisteraddress, cycleregisterdataon);
    uint16_t read2bytes = wiringPiI2CReadReg16(file_handle_ADC, conversionregisteraddress);
    float voltagevalue = convert_ad7994_output_to_volts(read2bytes);
    printf("(ALT)Reading value from conversion result register on ad7994-1 In1: %f\n",voltagevalue);
    error_code = wiringPiI2CWriteReg16(file_handle_ADC, configregisteraddress, configregisterdataon);
    error_code = wiringPiI2CWriteReg16(file_handle_ADC, cycleregisteraddress, cycleregisterdataon);
    return voltagevalue;
}*/
uint16_t read_ad7994_voltage(int channel) 
{
    uint8_t conversionregaddress;
    uint16_t read2bytes;
    if (channel == 1)
    {
        conversionregaddress = 0b00010000;
    }
    else if (channel == 2)
    {
        conversionregaddress = 0b00100000;
    }
    else if (channel == 3) 
    {
        conversionregaddress = 0b01000000;
    }
    else if (channel == 4)
    {
        conversionregaddress = 0b10000000;
    }
    else
    {
        conversionregaddress = 0b00010000;
    }
    //int errcode = wiringPiI2CWriteReg8(file_handle_ADC, configregisteraddress, config);
    //printf("Setting config register on ad7994-1 on In%d...   %d\n", channel, errcode);*/
    int errors = wiringPiI2CReadReg16(file_handle_ADC, conversionregaddress);
    if (errors == -1)
    {
        printf(" - UNREACHABLE\n");
    }
    float voltagevaluesum = 0;
    for (int i = 0; i < 3; i++)
    {
        read2bytes = wiringPiI2CReadReg16(file_handle_ADC, conversionregaddress);
        voltagevaluesum += convert_ad7994_output_to_volts(read2bytes);
        delay(10);
    }
    voltagevaluesum /= 3;
    printf("Reading value from conversion result register on ad7994-1 In%d: %f\n",channel, voltagevaluesum);
    return voltagevaluesum;
}
///
/// MAIN FUNCTION
///
 
int main (int argc, char **argv)
{    
    printf("Program started already.\n");
    system_init();
    // DRL
    TI_595_state_vector[65] = '0'; // do not drive DRL_SOURCE_SELECT
    TI_595_state_vector[63] = '1'; // drive DRL_JACK_SPST
    update_595_state();
    
    printf("Syncing...\n");
    for (int i=0; i<3;i++)
    {
        wait(SYNC_WAIT);
        digitalWrite(PI_IR_REC, HIGH);
        digitalWrite(PI_PHTRIG, HIGH);
        wait(SYNC_WAIT);
        digitalWrite(PI_IR_REC, LOW);
        digitalWrite(PI_PHTRIG, LOW);     
    }
    printf("Done.\n");
    // drive LARGE_SIGNAL_AC_SPST
    printf("Driving LARGE_SIGNAL_AC_SPST\n");
    TI_595_state_vector[69]='1';
    update_595_state();    
//Starting step1, sine measurements
    printf("Starting sine measurements\n");
    // set excitation DC
    // set DDS freq to 5 Hz (earlier 6)
    set_ad5696_dac_voltage(2.5, 1);
    dds_set_frequency(5);
    ac_dac_set(0.5);
    reset_relay_address();
    TI_595_state_vector[35]=0;
    TI_595_state_vector[36]=0;
    update_595_state();
    for(int i = 0; i <  ELECTRODES; i++) // for all electrodes excluding cz
    {
        // connect electrodes directly to driven
        TI_595_state_vector[relay_address]=1;
        TI_595_state_vector[relay_address+1]=1;
        update_595_state();
        printf("Measuring without resistance");
        wait(SINE_TIME); // wait for sine to finish
        // switching to measuring with 1MOhm resistance
        if (relay_address < 75) {
            TI_595_state_vector[relay_address]=0;
        }
        else {
            TI_595_state_vector[relay_address+1]=0;
        }
        update_595_state();
        wait(SINE_TIME);
        // connect electrodes back to common and resistance
        TI_595_state_vector[relay_address]=0;
        TI_595_state_vector[relay_address+1]=0;
        update_595_state();

        // next electrode
        if (i<17) printf("Selecting next electrode, now at address %d\n", relay_address);
        increment_relay_address(EXCLUDING_CZ);
    }
    ac_dac_set(0);
    set_ad5696_dac_voltage(0, 1);
    dds_set_frequency(0);
    printf("Done.\n"); 


// 1hz sine to ecg channel																										
	TI_595_state_vector[23]='1';  // Fp1 connected to COMMON via 0 Ohm
	TI_595_state_vector[24]='0';
	TI_595_state_vector[25]='1';  // Fp2 connected to COMMON via 0 Ohm
	TI_595_state_vector[26]='0';
	TI_595_state_vector[27]='1';  // F7 connected to COMMON via 0 Ohm
	TI_595_state_vector[28]='0';
	TI_595_state_vector[29]='1';  // F8 connected to COMMON via 0 Ohm
	TI_595_state_vector[30]='0';
	TI_595_state_vector[31]='1';  // C3 connected to COMMON via 0 Ohm
	TI_595_state_vector[32]='0';
	TI_595_state_vector[33]='1';  // C4 connected to COMMON via 0 Ohm
	TI_595_state_vector[34]='0';
	TI_595_state_vector[35]='1';  // Cz connected to COMMON via 0 Ohm
	TI_595_state_vector[36]='0';
	TI_595_state_vector[37]='1';  // F3 connected to COMMON via 0 Ohm
	TI_595_state_vector[38]='0';
	TI_595_state_vector[39]='1';  // F4 connected to COMMON via 0 Ohm
	TI_595_state_vector[40]='0';
	TI_595_state_vector[41]='1';  // Fz connected to COMMON via 0 Ohm
	TI_595_state_vector[42]='0';
	TI_595_state_vector[43]='1';  // T3 connected to COMMON via 0 Ohm
	TI_595_state_vector[44]='0';
	TI_595_state_vector[45]='1';  // T4 connected to COMMON via 0 Ohm
	TI_595_state_vector[46]='0';
	TI_595_state_vector[47]='1';  // P3 connected to COMMON via 0 Ohm
	TI_595_state_vector[48]='0';
	TI_595_state_vector[49]='1';  // Pz connected to COMMON via 0 Ohm
	TI_595_state_vector[50]='0';
	TI_595_state_vector[51]='1';  // P4 connected to COMMON via 0 Ohm
	TI_595_state_vector[52]='0';
	TI_595_state_vector[53]='1';  // O1 connected to COMMON via 0 Ohm
	TI_595_state_vector[54]='0';
	TI_595_state_vector[55]='1';  // O2 connected to COMMON via 0 Ohm
	TI_595_state_vector[56]='0';
	TI_595_state_vector[75]='0';  // T5 connected to COMMON via 0 Ohm
	TI_595_state_vector[76]='1';
	TI_595_state_vector[77]='0';  // T6 connected to COMMON via 0 Ohm
	TI_595_state_vector[78]='1';  
	update_595_state();

	printf("Starting 1Hz Sinewave to ECG channel\n");
    //parsing sine values from the table and setting the output voltage of ad5696 channel d to that value
    for (int j = 0; j < ECG_WAIT; j++)
    {
        for (int i = 0; i < 1000; i++)
        {
        error_code = wiringPiI2CWriteReg16(file_handle_DAC, 0b00111000, sinetable[i]);
        delayMicroseconds(108);//the time of setting the voltage is around 890microsecs
        }
    }
    error_code = wiringPiI2CWriteReg16(file_handle_DAC, 0b00111000, 0);
	printf("finished testing ECG with 1Hz sinewave\n");

///DRL test both ways
	/*printf("starting drl áramteszt both ways\n");
    //setting the state vector to the ideal setting in the header
    for (int i = 0; i < 80; i++) {
        TI_595_state_vector[i] = drltest[i];
    }
    update_595_state();
    set_ad5696_dac_voltage(2.5, 2);

    //measuring drl_cur_sig with excitation_dc set to 5, then 0
    while (1) {
    //excitation dc to 0
        set_ad5696_dac_voltage(5, 1);
        delay(100);
        read_ad7994_voltage(2);
        wait(5);
    //excitation dc to 5  
        set_ad5696_dac_voltage(0, 1);
        delay(100);
        read_ad7994_voltage(2);
        wait(5);
    }
	printf("finished drl áramteszt\n");
    printf("Resetting 595 state vector...");*/
    // status of the 595 chips are set to all zero 
    for (int i = 0; i < 80; i++)
    {
        TI_595_state_vector[i]= '0'; // clear the 595 state 
    }
    update_595_state();
    //resetting drl_cur_sense_bias
    set_ad5696_dac_voltage(0, 2);
    printf(" - OK\n");
    
    return 0;
}