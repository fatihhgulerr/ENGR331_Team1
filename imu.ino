#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

Adafruit_MPU6050 mpu;

void setup(void) {
  Serial2.begin(115200);
  RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
  ADC1->CR2 |= ADC_CR2_ADON;
  ADC1->SMPR1 |= ADC_SMPR1_SMP18_0 | ADC_SMPR1_SMP18_1 | ADC_SMPR1_SMP18_2; 
  ADC1->SQR3 = 18;
  ADC->CCR |= ADC_CCR_TSVREFE;
  Wire.setSDA(PB7);
  Wire.setSCL(PB6);
  Wire.begin();
  

  Serial2.println("Adafruit MPU6050 test!");

  if (!mpu.begin()) {
    Serial2.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial2.println("MPU6050 Found!");

  //setupt motion detection
  mpu.setHighPassFilter(MPU6050_HIGHPASS_0_63_HZ);
  mpu.setMotionDetectionThreshold(1);
  mpu.setMotionDetectionDuration(20);
  mpu.setInterruptPinLatch(true);	 
  mpu.setInterruptPinPolarity(true);
  mpu.setMotionInterrupt(true);

  Serial2.println("");
  delay(100);
}

void loop() {
  ADC1->CR2 |= ADC_CR2_SWSTART;
  while (!(ADC1->SR & ADC_SR_EOC));
  uint16_t adcValue = ADC1->DR;
  float V_sense = (adcValue * 3.3) / 4095.0;
  float Temperature = ((V_sense - 0.76) / 0.0025) + 25.0;

  if(mpu.getMotionInterruptStatus()) {
     
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

   
    Serial2.print("AccelX:");
    Serial2.print(a.acceleration.x);
    Serial2.print(",");
    Serial2.print("AccelY:");
    Serial2.print(a.acceleration.y);
    Serial2.print(",");
    Serial2.print("AccelZ:");
    Serial2.print(a.acceleration.z);
    Serial2.print(", ");
    Serial2.print("GyroX:");
    Serial2.print(g.gyro.x);
    Serial2.print(",");
    Serial2.print("GyroY:");
    Serial2.print(g.gyro.y);
    Serial2.print(",");
    Serial2.print("GyroZ:");
    Serial2.print(g.gyro.z);
    Serial2.println("");
  }
   Serial2.print("ADC Raw: ");
   Serial2.print(adcValue);
   Serial2.print(" | Voltage: ");
   Serial2.print(V_sense, 3);
   Serial2.print(" V | Temperature: ");
   Serial2.print(Temperature, 2);
   Serial2.println(" °C");

  delay(10);
}