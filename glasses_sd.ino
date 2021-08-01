#include <SdFat.h>

void setup_sd_card(){
	if(!Serial){
		Serial.begin(115200);
		while (!Serial);
	}

	pinMode(chip_select, OUTPUT);
	pinMode(chip_detect, INPUT);

	while (!digitalRead(chip_detect)) {
		Serial.println("No SD card detected");
		delay(1000);
	}

	if (!SD.begin(SD_CONFIG)) {
		Serial.println("initialization failed!");
		while (1);
	}

  if(SD.exists(mic_fname)){
    SD.remove(mic_fname);
  }
  if(SD.exists(imu_fname)){
    SD.remove(imu_fname);
  }

	mic_file = SD.open(mic_fname, O_WRITE | O_CREAT);
	imu_file = SD.open(imu_fname, O_WRITE | O_CREAT);

	card_present = digitalRead(chip_detect);

	attachInterrupt(digitalPinToInterrupt(chip_detect), card_detect, CHANGE);
}

void card_detect(){
	card_present = digitalRead(chip_detect);
	if(!card_present){
		setup_complete = 0;
	}
}
