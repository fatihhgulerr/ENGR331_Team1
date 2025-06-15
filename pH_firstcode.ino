/*
 * pH Sensörü - İki Noktalı Kalibrasyon
 *  pH = m·U + b
 *  m = (pH₂ − pH₁) / (U₂ − U₁)
 *  b = pH₁ − m·U₁
 */

const float PH_REF1 = 7.00;   // 1. tampon (pH 7)
const float U_REF1  = 0.114;  // pH 7’de okuduğun voltaj (V)

const float PH_REF2 = 8.00;   // 2. tampon (pH 8)
const float U_REF2  = 0.064;  // pH 8’de okuduğun voltaj (V)

/* Kalibrasyon katsayıları (derine takılma; bir kez hesaplayıp sabit bırak) */
const float SLOPE  = (PH_REF2 - PH_REF1) / (U_REF2 - U_REF1);   // m
const float OFFSET = PH_REF1 - SLOPE * U_REF1;                  // b

void setup() {
  Serial.begin(9600);
  analogReference(DEFAULT);          // 5 V referans (istenirse INTERNAL 1.1 V seçilebilir)
}

void loop() {
  /* 1) Ham değeri oku, gerilime çevir */
  int   raw     = analogRead(A0);
  float voltage = raw * (5.0 / 1023.0);

  /* 2) pH’yi hesapla */
  float pH = SLOPE * voltage + OFFSET;   // lineer kalibrasyon

  /* 3) Sonuçları yazdır */

  Serial.print("pH:");
  Serial.println(pH, 2);      // xx.xx

  delay(1000);
}
