#include <SPI.h>
#include <SD.h>
#include <Arduino_LSM9DS1.h>
#include <PDM.h>

#include <mbed.h>
#include <rtos.h>
#include <mbed_wait_api.h>
#include <platform/CircularBuffer.h>
#include <platform/Callback.h>

using namespace rtos;

#define IMU_FILE 0
#define MIC_FILE 1

/*************************Global Variables *************************/
// IMU data variables
float ax, ay, az, wx, wy, wz, mx, my, mz;
static const int imu_num_cum = 10;
float imu_data[9];
float imu_cum_data[imu_num_cum][9];
volatile int imu_counter, imu_data_rdy, imu_prev_time = 0;

// microphone variables
PDMClass PDM(D8, D7, D6);
static const char channels = 2;
static const int frequency = 41667;
short sampleBuffer[512];
volatile int samplesRead;
int mic_init = 0;
int mic_counter = 0;

// SD card variables
File imu_file, mic_file;
const int chipSelect = D10;
int resetOpenLog = D2;
volatile int file = MIC_FILE;

// threadding variables
Thread imu_thread;
/********************************************************************/

void setup_IMU() {
  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

  Serial.println("IMU initialized");
}

void update_IMU() {
  //   Serial.println("in update imu");
  imu_counter = imu_counter == imu_num_cum ? 0 : imu_counter;

  bool acc_avail, omg_avail, mag_avail;
  if (acc_avail = IMU.accelerationAvailable()) {
    IMU.readAcceleration(ax, ay, az);
  }

  if (omg_avail = IMU.gyroscopeAvailable()) {
    IMU.readGyroscope(wx, wy, wz);
  }

  if (mag_avail = IMU.magneticFieldAvailable()) {
    IMU.readMagneticField(mx, my, mz);
  }

  float temp_data[9] = {ax, ay, az, wx, wy, wz, mx, my, mz};
  for (int i = 0; i < 9; i++) {
    imu_data[i] = temp_data[i];
  }
  imu_counter++;
//  int start = millis();
//  Serial.println("PIMU: " + String(1000.0/(start - imu_prev_time)));
//  imu_prev_time = start;
  //  Serial.println("out update imu");
}

void print_IMU() {
  //  Serial.println("in print imu");
  //  noInterrupts();
  //  Serial.println("counter: " + String(imu_counter));
  if (imu_counter < imu_num_cum) return;
  //  if (appendFile("imu_data.csv")) {
  //    file = IMU_FILE;
  for (int i = 0; i < imu_num_cum; i++) {
    //      Serial.print("acc:\t" + String(imu_cum_data[i][0]) + "\t" + String(imu_cum_data[i][1]) + "\t" + String(imu_cum_data[i][2]) + "\t\t");
    //      Serial.print("ang:\t" + String(imu_cum_data[i][3]) + "\t" + String(imu_cum_data[i][4]) + "\t" + String(imu_cum_data[i][5]) + "\t\t");
    //      Serial.println("mag:\t" + String(imu_cum_data[i][6]) + "\t" + String(imu_cum_data[i][7]) + "\t" + String(imu_cum_data[i][8]) + "\t\t");
    Serial1.print("IMU, ");
    Serial1.print(String(imu_cum_data[i][0]) + ", " + String(imu_cum_data[i][1]) + ", " + String(imu_cum_data[i][2]) + ", ");
    Serial1.print(String(imu_cum_data[i][3]) + ", " + String(imu_cum_data[i][4]) + ", " + String(imu_cum_data[i][5]) + ", ");
    Serial1.println(String(imu_cum_data[i][6]) + ", " + String(imu_cum_data[i][7]) + ", " + String(imu_cum_data[i][8]) + ", ");
  }
  delay(5);
  //  }
  imu_counter = 0;
  imu_data_rdy = 0;
  //  Serial.println("out print imu");
  //  interrupts();
  int start = millis();
  Serial.println("PIMU: " + String(1000.0/(start - imu_prev_time)));
  imu_prev_time = start;
}

void imu_thread_job() {
  while (true) {
    for (int i = 0; i < imu_num_cum; i++) {
      update_IMU();
      for (int j = 0; j < 9; j++) {
        imu_cum_data[i][j] = imu_data[j];
      }
      delayMicroseconds(4000);
    }
    imu_data_rdy = 1;
    while (imu_data_rdy);
  }
}

void setup_mic() {
  // Configure the data receive callback
  PDM.onReceive(onPDMdata);

  // Optionally set the gain
  // Defaults to 20 on the BLE Sense and -10 on the Portenta Vision Shield
  // PDM.setGain(30);

  // Initialize PDM with:
  // - one channel (mono mode)
  // - a 16 kHz sample rate for the Arduino Nano 33 BLE Sense
  // - a 32 kHz or 64 kHz sample rate for the Arduino Portenta Vision Shield
  if (!PDM.begin(channels, frequency)) {
    Serial.println("Failed to start PDM!");
    while (1);
  }

  Serial.println("mic initialized");
  mic_init = 1;
  //  mic_thread.start(mbed::callback(print_mic));
}

/**
   Callback function to process the data from the PDM microphone.
   NOTE: This callback is executed as part of an ISR.
   Therefore using `Serial1` to print messages inside this function isn't supported.
 * */
void onPDMdata() {
  // Query the number of available bytes
  int bytesAvailable = PDM.available();

  // Read into the sample buffer
  PDM.read(sampleBuffer, bytesAvailable);

  // 16-bit, 2 bytes per sample
  samplesRead = bytesAvailable / 2;
}

void print_mic() {
  // Wait for samples to be read
  // Serial.println("Printing microphone data");
  //  int start = millis();
  if (imu_counter == imu_num_cum){
    print_IMU();
  }
  if (samplesRead) {
    //     Print samples to the Serial1 monitor or plotter
    //    if (file == MIC_FILE || appendFile("mic_data.csv")) {
    //      file = MIC_FILE;
    for (int i = 0; i < samplesRead / channels; i++) {
      if (channels == 2) {
        //            Serial.print("L:");
        Serial1.print("MIC, " + String(sampleBuffer[i]) + ", ");
        //            Serial.print(" R:");
        i++;
      }
      Serial1.println(sampleBuffer[i]);
    }
    samplesRead = 0;
    delay(5);
  }
  //  Serial.println("MIC: " + String(millis() - start));
}

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200);
  while (!Serial1 || !Serial);
  Serial.println("Started");

  //  setup_SD_card();
  setupOpenLog();
  setup_mic();
  setup_IMU();

  gotoCommandMode();
  runCommand("rm LOG*", '>');
  runCommand("rm imu_data.csv", '>');
  runCommand("rm mic_data.csv", '>');
  createFile("imu_data.csv");
  Serial1.println("ax, ay, az, wx, wy, wz, mx, my, mz");
  delay(10);
  createFile("mic_data.csv");
  Serial1.println("L, R");
  delay(10);
  appendFile("mic_data.csv");
  imu_thread.start(mbed::callback(imu_thread_job));
  Serial.println("done init");
}

void loop() {
  // update_IMU();
  print_IMU();
  print_mic();
  //  openlog_terminal();
}
