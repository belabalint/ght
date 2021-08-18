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

uint16_t sinetable[1000] = {51715, 53251, 54787, 56323, 57859, 59395, 61187, 62723, 64259, 260, 1796, 3332, 4868, 6404, 7940, 9476, 11012, 12548, 14084, 15876, 17412, 18948, 20484, 22020, 23556, 25092, 26628, 28164, 29700, 31236, 32772, 34308, 35844, 37380, 38916, 40452, 41988, 43268, 44804, 46340, 47876, 49412, 50948, 52484, 54020, 55556, 56836, 58372, 59908, 61444, 62980, 64516, 261, 1797, 3333, 4869, 6149, 7685, 9221, 10501, 12037, 13573, 14853, 16389, 17925, 19205, 20741, 22021, 23557, 24837, 26373, 27909, 29189, 30469, 32005, 33285, 34821, 36101, 37637, 38917, 40197, 41733, 43013, 44293, 45829, 47109, 48389, 49669, 50949, 52485, 53765, 55045, 56325, 57605, 58885, 60165, 61445, 62725, 64005, 65285, 1030, 2310, 3590, 4870, 6150, 7430, 8454, 9734, 11014, 12294, 13318, 14598, 15878, 16902, 18182, 19206, 20486, 21766, 22790, 24070, 25094, 26118, 27398, 28422, 29702, 30726, 31750, 32774, 34054, 35078, 36102, 37126, 38150, 39174, 40454, 41478, 42502, 43526, 44550, 45318, 46342, 47366, 48390, 49414, 50438, 51206, 52230, 53254, 54278, 55046, 56070, 56838, 57862, 58630, 59654, 60422, 61446, 62214, 62982, 64006, 64774, 7, 775, 1799, 2567, 3335, 4103, 4871, 5639, 6407, 7175, 7943, 8711, 9479, 9991, 10759, 11527, 12295, 12807, 13575, 14343, 14855, 15623, 16135, 16903, 17415, 18183, 18695, 19207, 19975, 20487, 20999, 21511, 22023, 22791, 23303, 23815, 24327, 24839, 25351, 25863, 26119, 26631, 27143, 27655, 27911, 28423, 28935, 29191, 29703, 30215, 30471, 30727, 31239, 31495, 32007, 32263, 32519, 32775, 33287, 33543, 33799, 34055, 34311, 34567, 34823, 35079, 35335, 35591, 35847, 35847, 36103, 36359, 36359, 36615, 36871, 36871, 37127, 37127, 37383, 37383, 37383, 37639, 37639, 37639, 37895, 37895, 37895, 37895, 37895, 37895, 37895, 37895, 37895, 37895, 37895, 37639, 37639, 37639, 37383, 37383, 37383, 37127, 37127, 36871, 36871, 36615, 36359, 36359, 36103, 35847, 35847, 35591, 35335, 35079, 34823, 34567, 34311, 34055, 33799, 33543, 33287, 32775, 32519, 32263, 32007, 31495, 31239, 30727, 30471, 30215, 29703, 29191, 28935, 28423, 27911, 27655, 27143, 26631, 26119, 25863, 25351, 24839, 24327, 23815, 23303, 22791, 22023, 21511, 20999, 20487, 19975, 19207, 18695, 18183, 17415, 16903, 16135, 15623, 14855, 14343, 13575, 12807, 12295, 11527, 10759, 9991, 9479, 8711, 7943, 7175, 6407, 5639, 4871, 4103, 3335, 2567, 1799, 775, 7, 64774, 64006, 62982, 62214, 61446, 60422, 59654, 58630, 57862, 56838, 56070, 55046, 54278, 53254, 52230, 51206, 50438, 49414, 48390, 47366, 46342, 45318, 44550, 43526, 42502, 41478, 40454, 39174, 38150, 37126, 36102, 35078, 34054, 32774, 31750, 30726, 29702, 28422, 27398, 26118, 25094, 24070, 22790, 21766, 20486, 19206, 18182, 16902, 15878, 14598, 13318, 12294, 11014, 9734, 8454, 7430, 6150, 4870, 3590, 2310, 1030, 65285, 64005, 62725, 61445, 60165, 58885, 57605, 56325, 55045, 53765, 52485, 50949, 49669, 48389, 47109, 45829, 44293, 43013, 41733, 40197, 38917, 37637, 36101, 34821, 33285, 32005, 30469, 29189, 27909, 26373, 24837, 23557, 22021, 20741, 19205, 17925, 16389, 14853, 13573, 12037, 10501, 9221, 7685, 6149, 4869, 3333, 1797, 261, 64516, 62980, 61444, 59908, 58372, 56836, 55556, 54020, 52484, 50948, 49412, 47876, 46340, 44804, 43268, 41988, 40452, 38916, 37380, 35844, 34308, 32772, 31236, 29700, 28164, 26628, 25092, 23556, 22020, 20484, 18948, 17412, 15876, 14084, 12548, 11012, 9476, 7940, 6404, 4868, 3332, 1796, 260, 64259, 62723, 61187, 59395, 57859, 56323, 54787, 53251, 51715, 50179, 48643, 47107, 45571, 44035, 42243, 40707, 39171, 37635, 36099, 34563, 33027, 31491, 29955, 28419, 26883, 25347, 23811, 22019, 20483, 18947, 17411, 15875, 14339, 12803, 11267, 9731, 8195, 6659, 5123, 3587, 2051, 515, 64514, 62978, 61442, 60162, 58626, 57090, 55554, 54018, 52482, 50946, 49410, 47874, 46594, 45058, 43522, 41986, 40450, 38914, 37634, 36098, 34562, 33026, 31746, 30210, 28674, 27394, 25858, 24322, 23042, 21506, 19970, 18690, 17154, 15874, 14338, 13058, 11522, 9986, 8706, 7426, 5890, 4610, 3074, 1794, 258, 64513, 63233, 61697, 60417, 59137, 57601, 56321, 55041, 53761, 52481, 50945, 49665, 48385, 47105, 45825, 44545, 43265, 41985, 40705, 39425, 38145, 36865, 35585, 34305, 33025, 31745, 30465, 29441, 28161, 26881, 25601, 24577, 23297, 22017, 20993, 19713, 18689, 17409, 16129, 15105, 13825, 12801, 11777, 10497, 9473, 8193, 7169, 6145, 5121, 3841, 2817, 1793, 769, 65280, 64256, 62976, 61952, 60928, 59904, 58880, 58112, 57088, 56064, 55040, 54016, 52992, 52224, 51200, 50176, 49152, 48384, 47360, 46592, 45568, 44800, 43776, 43008, 41984, 41216, 40448, 39424, 38656, 37888, 37120, 36096, 35328, 34560, 33792, 33024, 32256, 31488, 30720, 29952, 29184, 28416, 27904, 27136, 26368, 25600, 25088, 24320, 23552, 23040, 22272, 21760, 20992, 20480, 19712, 19200, 18688, 17920, 17408, 16896, 16384, 15872, 15104, 14592, 14080, 13568, 13056, 12544, 12032, 11776, 11264, 10752, 10240, 9984, 9472, 8960, 8704, 8192, 7680, 7424, 7168, 6656, 6400, 5888, 5632, 5376, 5120, 4608, 4352, 4096, 3840, 3584, 3328, 3072, 2816, 2560, 2304, 2048, 2048, 1792, 1536, 1536, 1280, 1024, 1024, 768, 768, 512, 512, 512, 256, 256, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 256, 256, 256, 512, 512, 512, 768, 768, 1024, 1024, 1280, 1536, 1536, 1792, 2048, 2048, 2304, 2560, 2816, 3072, 3328, 3584, 3840, 4096, 4352, 4608, 5120, 5376, 5632, 5888, 6400, 6656, 7168, 7424, 7680, 8192, 8704, 8960, 9472, 9984, 10240, 10752, 11264, 11776, 12032, 12544, 13056, 13568, 14080, 14592, 15104, 15872, 16384, 16896, 17408, 17920, 18688, 19200, 19712, 20480, 20992, 21760, 22272, 23040, 23552, 24320, 25088, 25600, 26368, 27136, 27904, 28416, 29184, 29952, 30720, 31488, 32256, 33024, 33792, 34560, 35328, 36096, 37120, 37888, 38656, 39424, 40448, 41216, 41984, 43008, 43776, 44800, 45568, 46592, 47360, 48384, 49152, 50176, 51200, 52224, 52992, 54016, 55040, 56064, 57088, 58112, 58880, 59904, 60928, 61952, 62976, 64256, 65280, 769, 1793, 2817, 3841, 5121, 6145, 7169, 8193, 9473, 10497, 11777, 12801, 13825, 15105, 16129, 17409, 18689, 19713, 20993, 22017, 23297, 24577, 25601, 26881, 28161, 29441, 30465, 31745, 33025, 34305, 35585, 36865, 38145, 39425, 40705, 41985, 43265, 44545, 45825, 47105, 48385, 49665, 50945, 52481, 53761, 55041, 56321, 57601, 59137, 60417, 61697, 63233, 64513, 258, 1794, 3074, 4610, 5890, 7426, 8706, 9986, 11522, 13058, 14338, 15874, 17154, 18690, 19970, 21506, 23042, 24322, 25858, 27394, 28674, 30210, 31746, 33026, 34562, 36098, 37634, 38914, 40450, 41986, 43522, 45058, 46594, 47874, 49410, 50946, 52482, 54018, 55554, 57090, 58626, 60162, 61442, 62978, 64514, 515, 2051, 3587, 5123, 6659, 8195, 9731, 11267, 12803, 14339, 15875, 17411, 18947, 20483, 22019, 23811, 25347, 26883, 28419, 29955, 31491, 33027, 34563, 36099, 37635, 39171, 40707, 42243, 44035, 45571, 47107, 48643, 50179};

// change this to 2 for quick testing, keep it 18 otherwise
#define ELECTRODES 2
 
// time constants in seconds
#define TRANSIENTS_WAIT             5
#define SYNC_WAIT                   0.5
#define NOISETEST_WAIT              10
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

unsigned char TI_595_state_vector[80]; 
 
// file handles for I2C devices
int file_handle_ADC, file_handle_DAC, file_handle_DigPot; 
  
// used to iterate through the electrode relays
// this variable is modified by increment_relay_address(char skip_cz)
// and also modified by reset_relay_address()
int relay_address = 23;

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
void reset_relay_address() // resets electrode address to FP1
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
void ac_dac_set(double m) // m is for multiplication factor [0, 1]
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
void wait(double s) // wait for s seconds
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
void connect_to_common(int address)
{
    if (address < 75)
    {
        TI_595_state_vector[address+1]='0';
    }
    else
    {
        TI_595_state_vector[address]='0';
    }
    update_595_state();
}
void connect_to_driven(int address)
{
    if (address < 75)
    {
        TI_595_state_vector[address+1]=1;
    }
    else
    {
        TI_595_state_vector[address]=1;
    }
    update_595_state();
}
void connect_to_R(int address)
{
    if (address < 75)
    {
        TI_595_state_vector[address]='0';
    }
    else
    {
        TI_595_state_vector[address+1]='0';
    }
    update_595_state();
}
void disconnect_from_R(int address)
{
    if (address < 75)
    {
        TI_595_state_vector[address]=1;
    }
    else
    {
        TI_595_state_vector[address+1]=1;
    }
    update_595_state();
}
void every_electrode_to_0Ohm_common()
{
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
}
void set_relays_to_drl_state()
{
    TI_595_state_vector[23]='1';  // Fp1 connected to driven via 0 Ohm
	TI_595_state_vector[24]='1';
	TI_595_state_vector[25]='1';  // Fp2 connected to DRIVEN via 0 Ohm
	TI_595_state_vector[26]='1';
	TI_595_state_vector[27]='1';  // F7 connected to DRIVEN via 0 Ohm
	TI_595_state_vector[28]='1';
	TI_595_state_vector[29]='1';  // F8 connected to DRIVEN via 0 Ohm
	TI_595_state_vector[30]='1';
	TI_595_state_vector[31]='1';  // C3 connected to DRIVEN via 0 Ohm
	TI_595_state_vector[32]='1';
	TI_595_state_vector[33]='1';  // C4 connected to DRIVEN via 0 Ohm
	TI_595_state_vector[34]='1';
	TI_595_state_vector[35]='1';  // Cz connected to DRIVEN via 0 Ohm
	TI_595_state_vector[36]='1';
	TI_595_state_vector[37]='1';  // F3 connected to DRIVEN via 0 Ohm
	TI_595_state_vector[38]='1';
	TI_595_state_vector[39]='1';  // F4 connected to DRIVEN via 0 Ohm
	TI_595_state_vector[40]='1';
	TI_595_state_vector[41]='1';  // Fz connected to DRIVEN via 0 Ohm
	TI_595_state_vector[42]='1';
	TI_595_state_vector[43]='1';  // T3 connected to DRIVEN via 0 Ohm
	TI_595_state_vector[44]='1';
	TI_595_state_vector[45]='1';  // T4 connected to DRIVEN via 0 Ohm
	TI_595_state_vector[46]='1';
	TI_595_state_vector[47]='1';  // P3 connected to DRIVEN via 0 Ohm
	TI_595_state_vector[48]='1';
	TI_595_state_vector[49]='1';  // Pz connected to DRIVEN via 0 Ohm
	TI_595_state_vector[50]='1';
	TI_595_state_vector[51]='1';  // P4 connected to DRIVEN via 0 Ohm
	TI_595_state_vector[52]='1';
	TI_595_state_vector[53]='1';  // O1 connected to DRIVEN via 0 Ohm
	TI_595_state_vector[54]='1';
	TI_595_state_vector[55]='1';  // O2 connected to DRIVEN via 0 Ohm
	TI_595_state_vector[56]='1';
	TI_595_state_vector[75]='1';  // T5 connected to DRIVEN via 0 Ohm
	TI_595_state_vector[76]='1';
	TI_595_state_vector[77]='1';  // T6 connected to DRIVEN via 0 Ohm
	TI_595_state_vector[78]='1';  
    TI_595_state_vector[65]='0';  //DRL_SOURCE_SELECT 
    TI_595_state_vector[63]='1';  //DRL_JACK_SPST
    TI_595_state_vector[62]='1';  //DRL_JACK_SPDT1
    TI_595_state_vector[73]='1';  //DRL_JACK_SPDT2
    TI_595_state_vector[68]='1';   //LARGE_VS_SMALL_SPDT
    TI_595_state_vector[69]='0';   //LARGE_SIGNAL_AC_SPST
    TI_595_state_vector[70]='0';   //COMMON_VS_MIX_SPDT
	update_595_state();
}
void system_shutdown()
{
    set_ad5696_dac_voltage(0, 0);
    set_ad5696_dac_voltage(0, 1);
    set_ad5696_dac_voltage(0, 2);
    set_ad5696_dac_voltage(0, 3);
    ac_dac_set(0);
    dds_set_frequency(0);
    printf("Resetting 595 state vector...");
    for (int i = 0; i < 80; i++)
    {
        TI_595_state_vector[i]= '0'; // clear the 595 state 
    }
    update_595_state();
}
void sine_to_ecg()
{
    int error_code;
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
}
void syncer()
{
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
}

int main (int argc, char **argv)
{    
    printf("Program started already.\n");
    system_init();
    syncer();
    // drive LARGE_SIGNAL_AC_SPST
    printf("Driving LARGE_SIGNAL_AC_SPST\n");
    TI_595_state_vector[69]='1';
    TI_595_state_vector[68]='0'; //large vs small spdt
    TI_595_state_vector[70]='0'; //common vs mix spdt
    update_595_state();    
//Step 0, noise measurements
    printf("Starting nosie measurements...\n");
    //connect drl to common
    TI_595_state_vector[65] = '0'; // do not drive DRL_SOURCE_SELECT
    TI_595_state_vector[63] = '1'; // drive DRL_JACK_SPST
    TI_595_state_vector[62] = '0'; //do not drive DRL_JACK_SPDT1
    update_595_state(); 
    every_electrode_to_0Ohm_common();
    wait(NOISETEST_WAIT);
//Starting step1, sine measurements
    printf("Starting sine measurements\n");
    // set excitation DC
    // set DDS freq to 5 Hz (earlier 6)
    set_ad5696_dac_voltage(2.5, 1);
    ac_dac_set(0.5);
    dds_set_frequency(5);
    for(int i = 0; i <  ELECTRODES; i++) // for all electrodes excluding cz
    {
        // connect electrodes directly to driven
        connect_to_driven(relay_address);
        disconnect_from_R(relay_address);
        wait(SINE_TIME);
        connect_to_R(relay_address);
        wait(SINE_TIME);
        connect_to_common(relay_address);
        // next electrode
        if (i<17) printf("Selecting next electrode, now at address %d\n", relay_address);
        increment_relay_address(EXCLUDING_CZ);
    }
    ac_dac_set(0);
    set_ad5696_dac_voltage(0, 1);
    dds_set_frequency(0);
    printf("Done.\n"); 


// 1hz sine to ecg channel																										

	//sine_to_ecg();

///DRL test both ways
	printf("starting drl áramteszt both ways\n");
    //setting the state vector to the ideal setting in the header
    set_relays_to_drl_state();
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
    system_shutdown();
    return 0;
}