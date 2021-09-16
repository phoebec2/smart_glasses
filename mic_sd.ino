#include <PDM.h>
#include <SdFat.h>
#include <sdios.h>

#define PDM_BUF_SIZE 1024 * 16
#define IMU_BUF_SIZE 1024 * 9

#ifndef SDCARD_SS_PIN
const uint8_t SD_CS_PIN = SS;
#else  // SDCARD_SS_PIN
// Assume built-in SD is used.
const uint8_t SD_CS_PIN = SDCARD_SS_PIN;
#endif  // SDCARD_SS_PIN

// Try max SPI clock for an SD. Reduce SPI_CLOCK if errors occur.
#define SPI_CLOCK SD_SCK_MHZ(50)

// Try to select the best SD card configuration.
#if HAS_SDIO_CLASS
#define SD_CONFIG SdioConfig(FIFO_SDIO)
#elif ENABLE_DEDICATED_SPI
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SPI_CLOCK)
#else  // HAS_SDIO_CLASS
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SPI_CLOCK)
#endif  // HAS_SDIO_CLASSs

/****************Global Variables***************/

// sd card variables
int chip_select = D10;
int chip_detect = D9;


SdExFat SD;
ExFile mic_file;
ExFile imu_file;

String mic_fname = "mic_data";
String imu_fname = "imu_data";

volatile int card_present = 0;


// pdm microphone variables
PDMClass PDM(D8, D7, D6);

int mic_prev_time, mic_flush_counter = 0;

// default number of output channels
static const char channels = 2;

// default PCM output frequency, 41667
static const int frequency = 41667;

// Buffer to read samples into, each sample is 16-bits
short mic_sampleBuffer[PDM_BUF_SIZE];
int pdm_sample_size = sizeof(mic_sampleBuffer[0]);
volatile int mic_print_arr;

// Number of audio samples read
volatile int samplesRead;


// imu variables
float ax, ay, az, wx, wy, wz, mx, my, mz;
float imu_sampleBuffer[IMU_BUF_SIZE];
int imu_buf_idx = 0;


// IR variables
volatile int IR_command_given = 0;

// others
volatile int setup_complete, int_mic, int_imu = 0;
/***********************************************/

void setup() {
	if(!Serial){
		Serial.begin(115200);
		while (!Serial);
	}

  pinMode(LED_BUILTIN, OUTPUT);
//	setup_sd_card();
//	setup_mic();
//	setup_imu();
  setup_ir();

	setup_complete = 1;
}

void loop() {
  IR_gesture_check();
	if(!card_present && setup_complete && IR_command_given){        // I added an ! in front of card_present
		save_mic_data();
		update_IMU();
    IR_command_given = 0;
    //digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
    //delay(1000);                       // wait for 1 seconds
    //digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
	} else if(card_present && !setup_complete){
		mic_file = SD.open(mic_fname, O_WRITE | O_CREAT);
		imu_file = SD.open(imu_fname, O_WRITE | O_CREAT);
		setup_complete = 1;
    	delay(100);
	} else {
		//Serial.println("SD card not present");
		//delay(250);
	}
}
