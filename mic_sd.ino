#include <PDM.h>
#include <SD.h>

#define PDM_BUF_SIZE 1024 * 32
#define IMU_BUF_SIZE 1024 * 9

/****************Global Variables***************/

// sd card variables
int chip_select = D10;
int chip_detect = D9;

File mic_file;
File imu_file;

String mic_fname = "mic_data";
String imu_fname = "imu_data";

volatile int card_present = 0;


// pdm microphone variables
PDMClass PDM(D8, D7, D6);

int mic_prev_time, mic_flush_counter = 0;

// default number of output channels
static const char channels = 1;

// default PCM output frequency, 41667
static const int frequency = 16000;

// Buffer to read samples into, each sample is 16-bits
short mic_sampleBuffer[2][PDM_BUF_SIZE];
volatile int mic_print_arr;

// Number of audio samples read
volatile int samplesRead;


// imu variables
float ax, ay, az, wx, wy, wz, mx, my, mz;
float imu_sampleBuffer[IMU_BUF_SIZE];
int imu_buf_idx = 0;


// others
volatile int setup_complete, int_mic, int_imu = 0;
/***********************************************/

void setup() {
	if(!Serial){
		Serial.begin(115200);
		while (!Serial);
	}

	setup_sd_card();
	setup_mic();
	setup_imu();

	setup_complete = 1;
}

void loop() {
	if(card_present && setup_complete){
		save_mic_data();
		update_IMU();
	} else if(card_present && !setup_complete){
		mic_file = SD.open(mic_fname, O_WRITE | O_CREAT);
		imu_file = SD.open(imu_fname, O_WRITE | O_CREAT);
		setup_complete = 1;
    	delay(100);
	} else {
		Serial.println("SD card not present");
		delay(250);
	}
}
