#include <PDM.h>
#include <SdFat.h>
#include <hal/nrf_pdm.h>

void setup_mic() {
  if (!Serial) {
    Serial.begin(115200);
    while (!Serial);
  }

  // Configure the data receive callback
  PDM.onReceive(onPDMdata);
  PDM.setBufferSize(PDM_BUF_SIZE * pdm_sample_size);

  mic_print_arr = 0;

  // Optionally set the gain
  // Defaults to 20 on the BLE Sense and -10 on the Portenta Vision Shield
  // PDM.setGain(30);

  if (!PDM.begin(channels, frequency)) {
    Serial.println("Failed to start PDM!");
    while (1);
  }
}

void save_mic_data() {
  // Wait for samples to be read
  if (samplesRead) {

//    int start = micros();
    int_mic = 0;
//    NVIC_DisableIRQ(PDM_IRQn);
    mic_file.write((byte*)mic_sampleBuffer, (size_t)(PDM_BUF_SIZE * pdm_sample_size));
//    NVIC_EnableIRQ(PDM_IRQn);
//    Serial.println("Bytes: " + String(PDM_BUF_SIZE * pdm_sample_size));
    if(int_mic){
      Serial.println("mic int");
    }
    if (++mic_flush_counter == 20) {
      mic_flush_counter = 0;
      mic_file.flush();
    }
    //		Serial.println("B/s: " + String(float(PDM_BUF_SIZE) / (float(micros() - start) / 1000000.0)));

//    int start = micros();
//    Serial.println("MIC freq: " + String(float(samplesRead) / (float(start - mic_prev_time) / 1000000.0)));
//    // Serial.println("MIC: " + String(samplesRead) + ", " + String(start - mic_prev_time) + "!!!!!!!!!!!!!!!");
//    mic_prev_time = start;

    // Clear the read count
    samplesRead = 0;
  }
}

/**
	 Callback function to process the data from the PDM microphone.
	 NOTE: This callback is executed as part of an ISR.
	 Therefore using `Serial` to print messages inside this function isn't supported.
 * */
void onPDMdata() {
  // Query the number of available bytes
  int bytesAvailable = PDM.available();

  // Read into the sample buffer
  PDM.read(mic_sampleBuffer, bytesAvailable);

  // 16-bit, 2 bytes per sample
  samplesRead = bytesAvailable / 2;

  int_mic = 1;
  int_imu = 1;
}
