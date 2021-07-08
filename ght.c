/*
  _    _ __ _   _  _____ _______ _____            
 | |  | /_ | \ | |/ ____|__   __|  __ \     /\    
 | |__| || |  \| | (___    | |  | |__) |   /  \   
 |  __  || | . ` |\___ \   | |  |  _  /   / /\ \  
 | |  | || | |\  |____) |  | |  | | \ \  / /  \ \ 
 |_|  |_||_|_| \_|_____/   |_|  |_|  \_\/_/    \_\
                                                  
          _____                    _____                _____          
         /\    \                  /\    \              /\    \         
        /::\    \                /::\____\            /::\    \        
       /::::\    \              /:::/    /            \:::\    \       
      /::::::\    \            /:::/    /              \:::\    \      
     /:::/\:::\    \          /:::/    /                \:::\    \     
    /:::/  \:::\    \        /:::/____/                  \:::\    \    
   /:::/    \:::\    \      /::::\    \                  /::::\    \   
  /:::/    / \:::\    \    /::::::\    \   _____        /::::::\    \  
 /:::/    /   \:::\ ___\  /:::/\:::\    \ /\    \      /:::/\:::\    \ 
/:::/____/  ___\:::|    |/:::/  \:::\    /::\____\    /:::/  \:::\____\
\:::\    \ /\  /:::|____|\::/    \:::\  /:::/    /   /:::/    \::/    /
 \:::\    /::\ \::/    /  \/____/ \:::\/:::/    /   /:::/    / \/____/ 
  \:::\   \:::\ \/____/            \::::::/    /   /:::/    /          
   \:::\   \:::\____\               \::::/    /   /:::/    /           
    \:::\  /:::/    /               /:::/    /    \::/    /            
     \:::\/:::/    /               /:::/    /      \/____/             
      \::::::/    /               /:::/    /                           
       \::::/    /               /:::/    /                            
        \::/____/                \::/    /                             
                                  \/____/                               
*/
 
/*
TABS ARE SPACES!
*/

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
 
// change this to 2 for quick testing, keep it 18 otherwise
#define ELECTRODES 2
 
// time constants in seconds
#define SHORT_MODE_WAIT             70
#define TRIANGLE_MODE_WAIT          4
#define SWEEP_MODE_WAIT             10
#define CMRR_WAIT                   70
#define TRANSIENTS_WAIT             5
#define BATEMU_WAIT                 0.1
#define SYNC_WAIT                   0.5
#define SINE_TIME                   3 
 
// you can use this for triangle/sweep mode waits:
// delayMicroseconds(4e6*(1/6.0)); // wait four periods
 
// the number of frequencies in sweep mode
#define FREQS   7
 
// F1, F2 (not used)
// used to generate logscale frequencies from F1 TO F2
#define F1      0.5
#define F2      50
 
// CMRR
#define CMRR_FREQ 50
 
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
 
// Kocka
#define TI_595_KOCKA_K4     67 
#define TI_595_KOCKA_K3     66
#define TI_595_KOCKA_K2     61
#define TI_595_KOCKA_SDI    12
#define TI_595_KOCKA_SYNC   13
#define TI_595_KOCKA_SCLK   14
 
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
typedef enum 
{
    DISABLE,
    ENABLE
} enable_t;
 
// waveform selector
typedef enum 
{
    SINE,
    TRIANGLE
} wave_t;
 
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
void reconnect_electrode_to_common(int address)
{

    printf("Reconnecting electrode to common\n");

    if (address < 75)
    {
        TI_595_state_vector[address]='1';     // drive spst
        TI_595_state_vector[address+1]='0';   // do not drive spdt
    }
    else
    {
        TI_595_state_vector[address]='0';     // do not drive spdt 
        TI_595_state_vector[address+1]='1';   // drive spst
    }
    update_595_state();
}
void connect_to_mohm(int address)
{
    if (relay_address < 75)
        {
            TI_595_state_vector[relay_address]='0';     // drive spst, connect 1MOhm
        }
    else
        {
            TI_595_state_vector[relay_address+1]='0';   // drive spst, connect 1MOhm
        }
    update_595_state();
}

void connect_electrode_to_driven(int address)
{
	printf("Connecting current electrode directly to driven\n");
    if (address < 75)
    {
        TI_595_state_vector[address]='1';     // drive spst
        TI_595_state_vector[address+1]='1';   // drive spdt
    }
    else
    {
        TI_595_state_vector[address]='1';     // drive spdt 
        TI_595_state_vector[address+1]='1';   // drive spst
    }
    update_595_state();
}
// resets electrode address to FP1
void reset_relay_address()
{
    relay_address = 23;
}
 
void USB_5v_out_enable(enable_t e)
{
    if(e == ENABLE)
    {
        printf("Enabling 5V USB out...");
        TI_595_state_vector[57] = '1';
    }
    else
    {
        printf("Disabling 5V USB out...");
        TI_595_state_vector[57] = '0';
    }
    printf(" - OK\n");
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
// input: SINE or TRIANGLE
void dds_init(wave_t waveform)
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
 
    TI_595_state_vector[TI_595_DDS_FSYNC] = '1';
    update_595_state();
    
    if (waveform == TRIANGLE)
    {
        dds_send(0b0010001000000010);
    }
    else
    {
        dds_send(0b0010001000000000);
    }
    
    TI_595_state_vector[TI_595_DDS_RESET]   = '0';
    update_595_state();
    
    TI_595_state_vector[TI_595_DDS_RESET]   = '1';
    update_595_state();
        
    TI_595_state_vector[TI_595_DDS_FSYNC] = '1';
    update_595_state();
    
    printf(" - OK\n");
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
 
    dds_init(SINE);
    ac_dac_init();
    ac_dac_set(0);
    kocka_init();
    
    printf("INIT END\n");
}
 
int AD5696R_set_voltage(double voltage, int channel)
{
    uint16_t dac_value;
    uint16_t tmp;
    uint8_t command;
    int error_code;
    
    if (channel == 0)
    {
        command = 0x31;
    }
    else if (channel == 1)
    {
        command = 0x32;
    }
    else 
    {
        command = 0x31;
    }
    
    // setting the output voltage 
    tmp = round((voltage/5.0)*(pow(2, 16)-1)); // (2^16)-1 = 65535
    
    dac_value = 0;
    dac_value = tmp << 8;
    dac_value = dac_value | (tmp >> 8);
        
    // writing DAC
    error_code = wiringPiI2CWriteReg16(file_handle_DAC, command, dac_value);
    
    return error_code;
}
 
void excitacion_dc_set_voltage(double voltage)
{
    int error_code;
    printf("Setting excitation DC voltage to %f V...", voltage);
    
    error_code = AD5696R_set_voltage(voltage, 1);
    
    if (error_code < 0) 
    {
        printf(" - FAILED\n"); 
    }
    else
    {
        printf(" - OK\n");
    }
}
 
void batemu_set_voltage(double voltage)
{
    int error_code;
    printf("Setting emulated battery voltage to %f V...", voltage);
    
    error_code = AD5696R_set_voltage(voltage, 0);
    
    if (error_code < 0) 
    {
        printf(" - FAILED\n"); 
    }
    else
    {
        printf(" - OK\n");
    }
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
///
/// MAIN FUNCTION
///
 
int main (int argc, char **argv)
{    
    /// for testing if the rasp GPIO pins are working properly (not guaranteed)
    if (wiringPiSetup () == -1)
        return 1;
    digitalWrite (21,1);        /// by T. Peti
    delay(2000);
    digitalWrite (21,0);
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
//Starting step1, sine measurements
    printf("Starting sine measurements\n");
    dds_init(SINE); // newly added
    // set excitation DC
    excitacion_dc_set_voltage(2.5);
    // set DDS freq to 5 Hz (earlier 6), sinewave already set
    dds_set_frequency(5); 
    // drive LARGE_SIGNAL_AC_SPST
    printf("Driving LARGE_SIGNAL_AC_SPST\n");
    TI_595_state_vector[69]='1';
    update_595_state();
    
    reset_relay_address();
    for(int i = 0; i <  ELECTRODES; i++) // for all electrodes excluding cz
    {
        // connect electrodes directly to driven
        connect_electrode_to_driven(relay_address);
        ac_dac_set(0.5);
        printf("Waiting for transients to settle...\n");
        wait(SINE_TIME); // wait for sine to finish
        ac_dac_set(0);
        // switching to measuring with 1MOhm resistance
        connect_to_mohm(relay_address);
        ac_dac_set(0.5);
        wait(SINE_TIME);
        ac_dac_set(0.0);    
        // connect electrodes back to common
        reconnect_electrode_to_common(relay_address);
        // next electrode
        if (i<17) printf("Selecting next electrode\n");
        increment_relay_address(EXCLUDING_CZ);
    }
    printf("Done.\n");


// 1hz sine to ecg channel
	printf("Starting 1Hz Sinewave to ECG channel");
	//connect to ecg channel
	###
	ac_dac_set(0.5);
	dds_set_frequency(1);
	printf("Sinewave starting on ECG channel...");
	wait(SINE_TIME);
	ac_dac_set(0);
	reconnect_electrode_to_common(#######);
	printf("finished testing ECG with 1Hz sinewave")


//DRL test both ways
	printf("starting drl áramteszt both ways")


	print("finished drl áramteszt")
    return 0 ;
}
 