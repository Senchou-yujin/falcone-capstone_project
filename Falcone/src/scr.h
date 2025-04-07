#include <Fuzzy.h>
#include <RTClib.h>
#include <SPI.h>
#include <Wire.h>
#include <SoftwareSerial.h>

#define SIM900_POWER_PIN 9  // Pin connected to SIM900's PWRKEY
SoftwareSerial mySerial(7, 8);  // SIM900 TX->D7, RX->D8

//PINS DESIGNATIONS
#define trigPin 5
#define echoPin 6
#define LED_PIN 10  // Define LED Indicator

// Define analog input pins
#define DO_PIN A2   // DO Sensor connected to A2
#define PH_PIN A0   // pH Sensor connected to A0

// Constants
#define VREF 5000      // Reference voltage in mV
#define ADC_RES 1024   // ADC Resolution

// Single-point calibration for DO sensor
#define CAL1_V (1600)  // Calibration voltage in mV
#define CAL1_T (25)    // Calibration temperature in °C

// pH Sensor Settings
#define OFFSET 0.00    // Adjust based on calibration
#define SAMPLING_INTERVAL 20
#define PRINT_INTERVAL 1000
#define ARRAY_LENGTH 40

// Height ng Sensor to riverbed
#define MAX_HEIGHT 250.0 // from sensor to riverbed

//Mobile Number of the Owner
String ownerNumber = "+639618936396";  // Owner's Number 09958919412
String researcherNumber = "+639295710236"; 

float doValue;
float pHValue;
int pHArray[ARRAY_LENGTH];   
int pHArrayIndex = 0;
static bool sentMorning = false;
static bool sentAfternoon = false;
String gateState = "Unknown";
long duration;
float distanceCM, distanceMeters;
float waterLevelCM, waterLevelMeters;
int waterSTAT = 0;
int lowTideAlertLevel = 0; 
int lastGateDecision = -1;  // Store the last gate decision

RTC_DS3231 rtc;
Fuzzy *fuzzy = new Fuzzy();

// Declare Fuzzy Inputs and Output globally
FuzzyInput *pH;
FuzzyInput *DO;
FuzzyOutput *WaterQuality;

// Data structure for tide schedule
typedef struct {
    uint32_t epoch;     // Time in seconds
    bool tideType;      // 1=High, 0=Low
} TideEntry;

// DO Lookup Table
const uint16_t DO_Table[41] = {
    14460, 14220, 13820, 13440, 13090, 12740, 12420, 12110, 11810, 11530,
    11260, 11010, 10770, 10530, 10300, 10080, 9860, 9660, 9460, 9270,
    9080, 8900, 8730, 8570, 8410, 8250, 8110, 7960, 7820, 7690,
    7560, 7430, 7300, 7180, 7070, 6950, 6840, 6730, 6630, 6530, 6410};

const TideEntry tideSchedule[] PROGMEM = {
    {1740869340, 1}, {1740959100, 0}, {1741004400, 0}, {1741092120, 0},
    {1741180140, 0}, {1741268700, 0}, {1741358100, 1}, {1741449600, 1},
    {1741542300, 1}, {1741633560, 1}, {1741723920, 1}, {1741813680, 1},
    {1741902960, 1}, {1741992060, 1}, {1742080980, 0}, {1742167020, 0},
    {1742210100, 0}, {1742296980, 0}, {1742384640, 0}, {1742472900, 0},
    {1742561640, 1}, {1742651100, 1}, {1742742000, 1}, {1742834820, 1},
    {1742926080, 1}, {1743016320, 1}, {1743106140, 1}, {1743195900, 1},
    {1743285780, 1}, {1743375960, 0}, {1743418560, 0}, {1743506340, 1},
    {1743594540, 1}, {1743683100, 1}, {1743772140, 1}, {1743862140, 1},
    {1743953940, 1}, {1744046280, 1}, {1744137840, 1}, {1744228560, 0},
    {1744318620, 0}, {1744408200, 0}, {1744497480, 0}, {1744538940, 0},
    {1744625460, 1}, {1744712340, 1}, {1744799940, 1}, {1744888020, 1},
    {1744976580, 1}, {1745065560, 1}, {1745155080, 1}, {1745245440, 1},
    {1745337000, 1}, {1745428380, 1}, {1745519460, 1}, {1745610660, 0},
    {1745701920, 0}, {1745746020, 1}, {1745833500, 1}, {1745921460, 1},
    {1746009780, 1}, {1746098460, 1}, {1746187560, 1}, {1746277140, 1},
    {1746367380, 1}, {1746458340, 1}, {1746549540, 0}, {1746641160, 0},
    {1746732780, 0}, {1746781080, 0}, {1746867720, 0}, {1746954600, 1},
    {1747041420, 1}, {1747128420, 1}, {1747215960, 1}, {1747390380, 1},
    {1747481160, 1}, {1747570260, 1}, {1747659660, 1}, {1747749480, 1},
    {1747839780, 1}, {1747930680, 1}, {1748023020, 0}, {1748074020, 0},
    {1748161140, 1}, {1748248860, 1}, {1748337000, 1}, {1748425560, 1},
    {1748514420, 1}, {1748603580, 1}, {1748693040, 1}, {1748782800, 1},
    {1748872560, 1}, {1748962200, 1}, {1749052080, 0}, {1749111240, 0},
    {1749195960, 0}, {1749282780, 1}, {1749369960, 1}, {1749457260, 1},
    {1749544740, 1}, {1749632460, 1}, {1749720480, 1}, {1749808800, 1},
    {1749897360, 1}, {1749986220, 1}, {1750075260, 1}, {1750164540, 1},
    {1750253940, 1}, {1750344240, 1}, {1750433940, 0}, {1750488000, 1},
    {1750575660, 1}, {1750663980, 1}, {1750752600, 1}, {1750841520, 1},
    {1750930680, 1}, {1751019960, 1}, {1751109360, 1}, {1751198820, 1},
    {1751288220, 1}, {1751377200, 1}, {1751465940, 0}, {1751554740, 0},
    {1751608440, 0}, {1751695920, 0}, {1751783940, 1}, {1751872260, 1},
    {1751960520, 1}, {1752048840, 1}, {1752137100, 1}, {1752225480, 1},
    {1752313920, 1}, {1752402600, 1}, {1752491460, 1}, {1752580500, 1},
    {1752669720, 1}, {1752759120, 1}, {1752848640, 0}, {1752899520, 1},
    {1752988320, 1}, {1753077960, 1}, {1753167720, 1}, {1753257360, 1},
    {1753346880, 1}, {1753436340, 1}, {1753525800, 1}, {1753615140, 1},
    {1753704360, 1}, {1753793280, 1}, {1753881900, 0}, {1753970400, 0},
    {1754016480, 0}, {1754104020, 0}, {1754193180, 1}, {1754283900, 1},
    {1754374560, 1}, {1754464200, 1}, {1754553240, 1}, {1754641920, 1},
    {1754730480, 1}, {1754819100, 1}, {1754907840, 1}, {1754996940, 1},
    {1755086220, 1}, {1755175740, 1}, {1755222120, 0}, {1755310260, 1},
    {1755399240, 1}, {1755489600, 1}, {1755581280, 1}, {1755672300, 1},
    {1755762660, 1}, {1755852540, 1}, {1755942120, 1}, {1756031400, 1},
    {1756120620, 1}, {1756209600, 1}, {1756298520, 0}, {1756384920, 0},
    {1756470780, 1}, {1756558440, 1}, {1756603620, 1}, {1756693920, 1},
    {1756784040, 1}, {1756876740, 1}, {1756967580, 1}, {1757057220, 1},
    {1757146320, 1}, {1757235240, 1}, {1757324160, 1}, {1757413440, 1},
    {1757503020, 1}, {1757592960, 1}, {1757678400, 1}, {1757766720, 1},
    {1757812260, 1}, {1757901660, 1}, {1757992860, 1}, {1758085560, 1},
    {1758177180, 1}, {1758267900, 1}, {1758357900, 1}, {1758447540, 1},
    {1758536880, 1}, {1758626040, 1}, {1758754620, 1}, {1758841680, 1}
};

// Function to calculate pH
float readPH() {
    int raw = analogRead(PH_PIN);  // Directly read ADC
    float voltage = (float)raw * VREF / ADC_RES / 1000.0;  // Convert to voltage
    return (3.5 * voltage) + OFFSET;  // Apply linear conversion
}


// Function to calculate DO in mg/L
float readDO() {
    int raw = analogRead(DO_PIN);
    float voltage = (float)raw * VREF / ADC_RES;  // Convert ADC to mV
    uint16_t V_saturation = CAL1_V + (35 * 25) - (CAL1_T * 35);  // 25°C fixed temp
    return (voltage * DO_Table[25] / V_saturation) / 1000.0;  // Convert to mg/L
}


// Turns SIM900 module ON
void powerOnSIM900() {
  digitalWrite(SIM900_POWER_PIN, HIGH);
  delay(1000);  // Required 1-second pulse
  digitalWrite(SIM900_POWER_PIN, LOW);

  delay(8000);  // Wait for module to initialize
}

// Check if SIM900 is responsive
bool checkSIM900Ready() {
  mySerial.println("AT");
  unsigned long startTime = millis();
  
  while (millis() - startTime < 2000) {  // Wait up to 2 seconds for a response
    if (mySerial.available()) {
      String response = mySerial.readString();
      Serial.println("Response: " + response);
      if (response.indexOf("OK") != -1) {
        return true;
      }
    }
  }
  return false;
}

// Function to handle missed call and SMS alert
void alertOwner(String number, String message) {
  Serial.println("Calling " + ownerNumber + "...");
  mySerial.println("ATD" + ownerNumber + ";");  // Dial the number
  delay(10000);  // Keep call active for 10 seconds (missed call)
  mySerial.println("ATH");  // Hang up the call
  Serial.println("Missed call sent!");

  delay(2000);  // Short delay before sending SMS

  Serial.println("Sending SMS to " + ownerNumber + "...");
  mySerial.print("AT+CMGS=\"");
  mySerial.print(ownerNumber);
  mySerial.println("\"");
  delay(500);

  mySerial.print(message);
  delay(500);
  mySerial.write(26); 
  delay(5000);

  Serial.println("SMS alert sent!");
}

void alertResearcher(String number, String message) {
  Serial.println("Calling " + researcherNumber + "...");
  mySerial.println("ATD" + researcherNumber + ";");  // Dial the number
  delay(10000);  // Keep call active for 10 seconds (missed call)
  mySerial.println("ATH");  // Hang up the call
  Serial.println("Missed call sent!");

  delay(2000);  // Short delay before sending SMS

  Serial.println("Sending SMS to " + researcherNumber + "...");
  mySerial.print("AT+CMGS=\"");
  mySerial.print(researcherNumber);
  mySerial.println("\"");
  delay(500);

  mySerial.print(message);
  delay(500);
  mySerial.write(26); 
  delay(5000);

  Serial.println("SMS alert sent!");
}

// Function to extract date components from an epoch timestamp
void getDateFromEpoch(uint32_t epoch, int &year, int &month, int &day) {
    DateTime dt = DateTime(epoch);
    year = dt.year();
    month = dt.month();
    day = dt.day();
}

// Function to check for all tide events within the current day
void checkFutureLowTides(uint32_t currentEpoch) {
    size_t startIdx = 0;
    bool foundStart = false;

    // Find the first future tide event (from today onward)
    for (size_t i = 0; i < sizeof(tideSchedule) / sizeof(TideEntry); i++) {
        TideEntry entry;
        memcpy_P(&entry, &tideSchedule[i], sizeof(TideEntry));

        if (entry.epoch >= currentEpoch) {
            startIdx = i;
            foundStart = true;
            break;
        }
    }

    if (!foundStart) {
        return;
    }

    Serial.println("Checking consecutive low tides from today onward...");

    // Scan forward in the tide dataset
    int lowTideCount = 0; 
    const size_t scanLimit = 6;
    size_t scanCounter = 0; 

    for (size_t i = startIdx; i < sizeof(tideSchedule) / sizeof(TideEntry); i++) {
        if (scanCounter >= scanLimit) break;

        TideEntry entry;
        memcpy_P(&entry, &tideSchedule[i], sizeof(TideEntry));

        if (entry.tideType == 0) {
        } else { 
            lowTideCount = 0; 
        }

        // Print each detected tide
        Serial.print("Epoch: ");
        Serial.print(entry.epoch);
        Serial.print(" | Tide: ");
        Serial.println(entry.tideType ? "High" : "Low");

        // Trigger alerts
        if (lowTideCount == 3) {
            lowTideAlertLevel = 3;
        } else if (lowTideCount == 5) {
            lowTideAlertLevel = 5;
            break; 
        }
        scanCounter++; 
    }
    // Serial.print("Final Consecutive Low Tides Count: ");
    // Serial.println(lowTideCount);
}

float getWaterLevel() {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    duration = pulseIn(echoPin, HIGH);
    
    distanceCM = (duration * 0.034 / 2); // Distance in cm
    distanceMeters = distanceCM / 100.0; // Convert cm to meters

    // Calculate actual water level
    waterLevelCM = MAX_HEIGHT - distanceCM;
    waterLevelMeters = waterLevelCM / 100.0;

    return waterLevelMeters;
}

// Function to determine tide status
String getTideStatus(float waterLevelMeters) {
    if (waterLevelMeters >= 0 && waterLevelMeters <= 0.8) return "Low";
    else if (waterLevelMeters >= 0.81 && waterLevelMeters <= 2) return "High";
    else return "Unknown";
}

// Function to classify water quality and set waterSTAT
String classifyWaterQuality(float waterQuality) {
    if (waterQuality >= 0 && waterQuality <= 25) {
        waterSTAT = 1;
        return "Toxic (T)";
    } else if (waterQuality > 25 && waterQuality <= 50) {
        waterSTAT = 2;
        return "Poor (P)";
    } else if (waterQuality > 50 && waterQuality <= 75) {
        waterSTAT = 3;
        return "Good (G)";
    } else if (waterQuality > 75 && waterQuality <= 100) {
        waterSTAT = 4;
        return "Excellent (E)";
    } else {
        waterSTAT = 0;
        return "Unknown";
    }
}

// Water Quality + Tide Status = Gate Decision
int getGateDecision(int waterSTAT, String tideStatus) {
    int gateState;
    if ((waterSTAT == 1) && (tideStatus == "Low")) gateState = 11;                 // Toxic + Low  = Open 
    else if ((waterSTAT == 1) && (tideStatus == "High")) gateState = 12;           // Toxic + High = Open 

    else if ((waterSTAT == 2) && (tideStatus == "Low")) gateState = 21;            // Poor + Low   = Conditional 
    else if ((waterSTAT == 2) && (tideStatus == "High")) gateState = 22;           // Poor + High  = Open 

    else if ((waterSTAT == 3) && (tideStatus == "Low")) gateState = 31;            // Good + Low   = Close 
    else if ((waterSTAT == 3) && (tideStatus == "High")) gateState = 32;           // Good + High = Conditional 

    else if ((waterSTAT == 4) && (tideStatus == "Low")) gateState = 41;            // Excellent + Low  = Close 
    else if ((waterSTAT == 4) && (tideStatus == "High")) gateState = 42;           // Excellent + High = Close 
    else gateState = -1;
    return gateState;
}

// Function to blink LED
void blinkLED(int times, int interval) {
    for (int i = 0; i < times; i++) {
        digitalWrite(LED_PIN, HIGH);
        delay(interval);
        digitalWrite(LED_PIN, LOW);
        delay(interval);
    }
}

// Function to fade LED in and out
void fadeLED(int times) {
    for (int j = 0; j < times; j++) {
        for (int brightness = 0; brightness <= 255; brightness++) {
            analogWrite(LED_PIN, brightness);  // Fade in
            delay(5);
        }
        for (int brightness = 255; brightness >= 0; brightness--) {
            analogWrite(LED_PIN, brightness);  // Fade out
            delay(5);
        }
    }
}

void setup() {
    Serial.begin(9600);
    mySerial.begin(9600);
    pinMode(SIM900_POWER_PIN, OUTPUT);
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    pinMode(LED_PIN, OUTPUT);
     // Initialize RTC
    Wire.begin();  // Uses hardware I2C (SDA = 20, SCL = 21)
    if (!rtc.begin()) {
        Serial.println("Couldn't find RTC");
        blinkLED(3,500);
        while (1);
    }

    if (rtc.lostPower()) {
        Serial.println("RTC lost power, setting time...");
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // Set time to compile time
    }

    // Initialize FuzzyInput for pH level
    pH = new FuzzyInput(1);
    FuzzySet *acidic = new FuzzySet(-1, 2, 5.4, 6.3); 
    FuzzySet *normal = new FuzzySet(6.2, 7, 8.3, 8.5); 
    FuzzySet *alkaline = new FuzzySet(8.4, 9.5, 14, 15); 


    pH->addFuzzySet(acidic);
    pH->addFuzzySet(normal);
    pH->addFuzzySet(alkaline);
    fuzzy->addFuzzyInput(pH);

    // Initialize FuzzyInput for DO (Dissolved Oxygen)
    DO = new FuzzyInput(2);
    FuzzySet *veryLow = new FuzzySet(-1, 1, 2.4, 4.2); 
    FuzzySet *low = new FuzzySet(3.8, 5, 7.8, 8.2); 
    FuzzySet *normalDO = new FuzzySet(7.8, 9, 18, 21);
    
    DO->addFuzzySet(veryLow);
    DO->addFuzzySet(low);
    DO->addFuzzySet(normalDO);
    fuzzy->addFuzzyInput(DO);

    // Initialize FuzzyOutput for Water Quality based on table
    WaterQuality = new FuzzyOutput(1);
    FuzzySet *toxic = new FuzzySet(0, 12.5, 12.5, 25);
    FuzzySet *poor = new FuzzySet(26, 38, 38, 50);
    FuzzySet *good = new FuzzySet(51, 63, 63, 75);
    FuzzySet *excellent = new FuzzySet(76, 88, 88, 100);
    
    WaterQuality->addFuzzySet(toxic);
    WaterQuality->addFuzzySet(poor);
    WaterQuality->addFuzzySet(good);
    WaterQuality->addFuzzySet(excellent);
    fuzzy->addFuzzyOutput(WaterQuality);

    // Define Fuzzy Rules

    // (1) Acidic + Very Low DO → Toxic 
    FuzzyRuleAntecedent *acidic_veryLow = new FuzzyRuleAntecedent();
    acidic_veryLow->joinWithAND(acidic, veryLow);
    FuzzyRuleConsequent *toxic_output = new FuzzyRuleConsequent();
    toxic_output->addOutput(toxic);
    fuzzy->addFuzzyRule(new FuzzyRule(1, acidic_veryLow, toxic_output));

    // (2) Acidic + Low DO → Poor
    FuzzyRuleAntecedent *acidic_low = new FuzzyRuleAntecedent();
    acidic_low->joinWithAND(acidic, low);
    FuzzyRuleConsequent *poor_output = new FuzzyRuleConsequent();
    poor_output->addOutput(poor);
    fuzzy->addFuzzyRule(new FuzzyRule(2, acidic_low, poor_output));

    // (3) Acidic + Normal DO → Good
    FuzzyRuleAntecedent *acidic_normal = new FuzzyRuleAntecedent();
    acidic_normal->joinWithAND(acidic, normalDO);
    FuzzyRuleConsequent *good_output = new FuzzyRuleConsequent();
    good_output->addOutput(good);
    fuzzy->addFuzzyRule(new FuzzyRule(3, acidic_normal, good_output));

    // (4) Normal pH + Very Low DO → Poor 
    FuzzyRuleAntecedent *normal_veryLow = new FuzzyRuleAntecedent();
    normal_veryLow->joinWithAND(normal, veryLow);
    fuzzy->addFuzzyRule(new FuzzyRule(4, normal_veryLow, poor_output));

    // (5) Normal pH + Low DO → Excellent
    FuzzyRuleAntecedent *normal_low = new FuzzyRuleAntecedent();
    normal_low->joinWithAND(normal, low);
    FuzzyRuleConsequent *excellent_output = new FuzzyRuleConsequent();
    excellent_output->addOutput(excellent);
    fuzzy->addFuzzyRule(new FuzzyRule(5, normal_low, excellent_output));

    // (6) Normal pH + Normal DO → Excellent
    FuzzyRuleAntecedent *normal_normal = new FuzzyRuleAntecedent();
    normal_normal->joinWithAND(normal, normalDO);
    fuzzy->addFuzzyRule(new FuzzyRule(6, normal_normal, excellent_output));

    // (7) Alkaline + Very Low DO → Toxic
    FuzzyRuleAntecedent *alkaline_veryLow = new FuzzyRuleAntecedent();
    alkaline_veryLow->joinWithAND(alkaline, veryLow);
    fuzzy->addFuzzyRule(new FuzzyRule(7, alkaline_veryLow, toxic_output));

    // (8) Alkaline + Low DO → Poor 
    FuzzyRuleAntecedent *alkaline_low = new FuzzyRuleAntecedent();
    alkaline_low->joinWithAND(alkaline, low);
    fuzzy->addFuzzyRule(new FuzzyRule(8, alkaline_low, poor_output));

    // (9) Alkaline + Normal DO → Good 
    FuzzyRuleAntecedent *alkaline_normal = new FuzzyRuleAntecedent();
    alkaline_normal->joinWithAND(alkaline, normalDO);
    fuzzy->addFuzzyRule(new FuzzyRule(9, alkaline_normal, good_output)); 

    // Get current time
    DateTime now = rtc.now();
    uint32_t currentEpoch = now.unixtime();

    Serial.println("Initializing GSM module...");

      // // Check if SIM900 responds to AT command
      // if (checkSIM900Ready()) {
      //   Serial.println("SIM900 is ON.");
      // } else {
      //   Serial.println("SIM900 Powering ON...");
      //   powerOnSIM900();

      //   // Double-check after power-on
      //   if (checkSIM900Ready()) {
      //     Serial.println("SIM900 is now responsive.");
      //   } else {
      //     Serial.println("Check connections.");
      //   }
      // }
    
    // mySerial.println("AT");
    // delay(500);
    // if (!mySerial.find("OK")) {  // If "OK" is NOT received
    //     Serial.println("Error: GSM not responding!");
    //     blinkLED(4, 500);  // Blink LED 4 times (500ms interval)
    //     return;
    // }
    mySerial.println("AT+CMGF=1");
    delay(500);
    mySerial.println("AT+CSCS=\"GSM\"");
    delay(500);

    Serial.println("GSM module ready!");

    // Process today's tide events
    checkFutureLowTides(currentEpoch);
    Serial.print("Low tide Count: ");
    Serial.println(lowTideAlertLevel);
}

void loop() {
    pHValue = readPH();
    doValue = readDO();
            
    // Set Inputs
    fuzzy->setInput(1, pHValue);
    fuzzy->setInput(2, doValue);

    // Evaluate Fuzzy Logic
    fuzzy->fuzzify();

    // Get Fuzzy Output
    float waterQuality = fuzzy->defuzzify(1);
    String qualityLabel = classifyWaterQuality(waterQuality);

    // Simulate Tide Data
    waterLevelMeters = getWaterLevel();
    String tideStatus = getTideStatus(waterLevelMeters);
    int gateDecision = getGateDecision(waterSTAT, tideStatus);
    String gateText;
    switch (gateDecision) {
        case 11: case 12: gateText = "Open"; break;
        case 21: case 32: gateText = "Conditional"; break;
        case 22: gateText = "Open"; break;
        case 31: case 41: case 42: gateText = "Close"; break;
        default: gateText = "Unknown"; break;
    }

    // Debug Output
    Serial.println("------------------------------------------------------");
    Serial.print("pH: "); Serial.print(pHValue, 2);
    Serial.print(" | DO: "); Serial.print(doValue, 2);
    Serial.print(" | Water Quality: "); Serial.print(waterQuality);
    Serial.print(" | Classification: "); Serial.println(qualityLabel);
    
    Serial.print("Tide Height: "); Serial.print(waterLevelMeters, 2); Serial.print(" m");
    Serial.print(" | Tide Status: "); Serial.println(tideStatus);
    
    Serial.print("Gate Decision: "); Serial.println(gateText);
    Serial.println("------------------------------------------------------");

    // Get current time
    DateTime now = rtc.now();
    int currentHour = now.hour();
    int currentMinute = now.minute();

    // Send Daily status to OWNER
    // Check for 7:00 AM alert
    if (currentHour == 7 && currentMinute == 0 && !sentMorning) {    // ownerNumber = "+63XXXXXXXXXX"
        Serial.println("ALERT: It's Time!");
        Serial.println("Calling +639618936396...");
        mySerial.println("ATD+639618936396;");  // Dial number
        delay(10000);  // Keep call active for 10 secs
        mySerial.println("ATH");  // Hang up the call
        Serial.println("Missed call sent!");

        delay(2000);  

        Serial.println("Sending SMS to +639618936396...");
        mySerial.print("AT+CMGS=\"+639618936396\"\r");
        delay(500);

        mySerial.print("STATUS NG TUBIG\nDO: " + String(doValue) + 
                        "\npH: " + String(pHValue) + 
                        "\nWater Height: " + String(waterLevelMeters) + "m");
        delay(500);
        mySerial.write(26); 
        delay(5000);

        Serial.println("SMS alert sent!");

        sentMorning = true;
    }
    if (currentHour == 7 && currentMinute > 3) {
        sentMorning = false;  // Reset flag after 7:03 AM has passed
    }

        // Check for 12:00 AM alert
    if (currentHour == 12 && currentMinute == 0 && !sentMorning) {    // ownerNumber = "+63XXXXXXXXXX"
        Serial.println("ALERT: It's Time!");
        Serial.println("Calling +639618936396...");
        mySerial.println("ATD+639618936396;");  // Dial number
        delay(10000);  // Keep call active for 10 secs
        mySerial.println("ATH");  // Hang up the call
        Serial.println("Missed call sent!");

        delay(2000);  

        Serial.println("Sending SMS to +639618936396...");
        mySerial.print("AT+CMGS=\"+639618936396\"\r");
        delay(500);

        mySerial.print("STATUS NG TUBIG\nDO: " + String(doValue) + 
                        "\npH: " + String(pHValue) + 
                        "\nWater Height: " + String(waterLevelMeters) + "m");
        delay(500);
        mySerial.write(26); 
        delay(5000);

        Serial.println("SMS alert sent!");

        sentMorning = true;
    }
    if (currentHour == 12 && currentMinute > 3) {
        sentMorning = false;  // Reset flag after 12:03 AM has passed
    }

    // Check for 5:00 PM alert
    if (currentHour == 17 && currentMinute == 0 && !sentAfternoon) {
        Serial.println("ALERT: It's Time!");
        Serial.println("Calling +639618936396...");
        mySerial.println("ATD+639618936396;");  // Dial the number
        delay(10000);  // Keep call active for 10 secs
        mySerial.println("ATH");  // Hang up the call
        Serial.println("Missed call sent!");

        delay(2000); 

        Serial.println("Sending SMS to +639618936396...");
        mySerial.print("AT+CMGS=\"+639618936396\"\r");
        delay(500);

        mySerial.print("STATUS NG TUBIG\nDO: " + String(doValue) + 
                        "\npH: " + String(pHValue) + 
                        "\nWater Height: " + String(waterLevelMeters) + "m");
        delay(500);
        mySerial.write(26); 
        delay(5000);

        Serial.println("SMS alert sent!");
        sentAfternoon = true;
    }
    if (currentHour == 17 && currentMinute > 3) {
        sentAfternoon = false; 
    }

    // Send Daily Status to Researchers
    // Check for 7:00 AM alert
    if (currentHour == 7 && currentMinute == 5 && !sentMorning) {    // ownerNumber = "+63XXXXXXXXXX"
        Serial.println("ALERT: It's Time!");
        Serial.println("Calling +639958919412...");
        mySerial.println("ATD+639958919412;");  // Dial number
        delay(10000);  // Keep call active for 10 secs
        mySerial.println("ATH");  // Hang up the call
        Serial.println("Missed call sent!");

        delay(2000);  

        Serial.println("Sending SMS to +639958919412...");
        mySerial.print("AT+CMGS=\"+639958919412\"\r");
        delay(500);

        mySerial.print("STATUS NG TUBIG\nDO: " + String(doValue) + 
                        "\npH: " + String(pHValue) + 
                        "\nWater Height: " + String(waterLevelMeters) + "m");
        delay(500);
        mySerial.write(26); 
        delay(5000);

        Serial.println("SMS alert sent!");

        sentMorning = true;
    }
    if (currentHour == 7 && currentMinute > 10) {
        sentMorning = false;  
    }

        // Check for 12:00 AM alert
    if (currentHour == 12 && currentMinute == 5 && !sentMorning) {    // ownerNumber = "+63XXXXXXXXXX"
        Serial.println("ALERT: It's Time!");
        Serial.println("Calling +639958919412...");
        mySerial.println("ATD+639958919412;");  // Dial number
        delay(10000);  // Keep call active for 10 secs
        mySerial.println("ATH");  // Hang up the call
        Serial.println("Missed call sent!");

        delay(2000);  

        Serial.println("Sending SMS to +639958919412...");
        mySerial.print("AT+CMGS=\"+639958919412\"\r");
        delay(500);

        mySerial.print("STATUS NG TUBIG\nDO: " + String(doValue) + 
                        "\npH: " + String(pHValue) + 
                        "\nWater Height: " + String(waterLevelMeters) + "m");
        delay(500);
        mySerial.write(26); 
        delay(5000);

        Serial.println("SMS alert sent!");

        sentMorning = true;
    }
    if (currentHour == 12 && currentMinute > 10) {
        sentMorning = false;  // Reset flag after 8:00 AM has passed
    }

    // Check for 5:00 PM alert
    if (currentHour == 17 && currentMinute == 5 && !sentAfternoon) {
        Serial.println("ALERT: It's Time!");
        Serial.println("Calling +639958919412...");
        mySerial.println("ATD+639958919412;");  // Dial the number
        delay(10000);  // Keep call active for 10 secs
        mySerial.println("ATH");  // Hang up the call
        Serial.println("Missed call sent!");

        delay(2000); 

        Serial.println("Sending SMS to +639958919412...");
        mySerial.print("AT+CMGS=\"+639958919412\"\r");
        delay(500);

        mySerial.print("STATUS NG TUBIG\nDO: " + String(doValue) + 
                        "\npH: " + String(pHValue) + 
                        "\nWater Height: " + String(waterLevelMeters) + "m");
        delay(500);
        mySerial.write(26); 
        delay(5000);

        Serial.println("SMS alert sent!");
        sentAfternoon = true;
    }
    if (currentHour == 17 && currentMinute > 10) {
        sentAfternoon = false; 
    }

    // Send SMS Alert 
    sendAlert(gateDecision, lowTideAlertLevel);

    delay(60000); //Check All Parameters in 1 Min (60 Seconds)
}

void sendAlert(int gateDecision, int lowTideAlertLevel) {

    // Skip sending alert if the gate decision is unknown
    if (gateDecision == -1) {
        Serial.println("No alert sent");
        
        return;
    }

    // Only send an alert if the gateDecision has changed
    if (gateDecision != lastGateDecision) {
        String message;

        switch (gateDecision) {
            case 11: message = "ALERT: Toxic Water habang Low Tide → OPEN Gate"; break;
            case 12: message = "ALERT: Toxic Water habang High Tide → OPEN Gate"; break;
            case 21:  
                if (lowTideAlertLevel >= 3) {
                    message = "ALERT: Poor Water habang Low Tide (3-day consecutive) → OPEN Gate";
                } else {
                    message = "ALERT: Poor Water habang Low Tide → CLOSE Gate"; 
                }
                break;
            case 22: message = "ALERT: Poor Water habang High Tide → OPEN Gate"; break;
            case 31: message = "ALERT: Good Water habang Low Tide → CLOSE Gate"; break;
            case 32:
                if (lowTideAlertLevel >= 5) {
                    message = "ALERT: Good Water habang High Tide (5-day consecutive Low Tides) → OPEN Gate";
                } else {
                    message = "ALERT: Good Water habang High Tide → CLOSE Gate";
                }
                break;
            case 41: message = "ALERT: Excellent Water habang Low Tide → CLOSE Gate"; break;
            case 42: message = "ALERT: Excellent Water habang High Tide → CLOSE Gate"; break;
            default: Serial.println("No alert sent: Unknown gate state."); return;
        }

        Serial.println(message);
        // Send missed call and SMS alert
        Serial.println("Sending to Owner: ");
        alertOwner(ownerNumber, message);
        //send status to owner
        Serial.println("Sending Status: ");
        Serial.println("Sending SMS to +639618936396...");
        mySerial.print("AT+CMGS=\"+639618936396\"\r");
        delay(500);

        mySerial.print("STATUS NG TUBIG\nDO: " + String(doValue) + 
                        "\npH: " + String(pHValue) + 
                        "\nWater Height: " + String(waterLevelMeters) + "m");
        delay(500);
        mySerial.write(26); 
        delay(5000);

        Serial.println("SMS alert sent!");

        delay(5000);


        Serial.println("Sending to Researchers: ");
        alertResearcher(researcherNumber, message);
        //Send status to researcher
        Serial.println("Sending SMS to +639295710236..."); //639958919412
        mySerial.print("AT+CMGS=\"+639295710236\"\r");
        delay(500);

        mySerial.print("STATUS NG TUBIG\nDO: " + String(doValue) + 
                        "\npH: " + String(pHValue) + 
                        "\nWater Height: " + String(waterLevelMeters) + "m");
        delay(500);
        mySerial.write(26); 
        delay(5000);

        Serial.println("SMS alert sent!");

        delay(5000);

        // Update lastGateDecision to prevent duplicate alerts
        lastGateDecision = gateDecision;
    }
}