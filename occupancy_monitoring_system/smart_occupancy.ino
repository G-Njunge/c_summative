#include <LiquidCrystal.h>
#include <stdlib.h>

/* Pin assignments */
const int ENTRY_PIN = 4;
const int EXIT_PIN = 5;
const int GREEN_LED = 2;
const int RED_LED = 3;

/* Timing in milliseconds */
const unsigned long DEBOUNCE_MS = 50;
const unsigned long LCD_REFRESH_MS = 500;

/* Facility limit (kept small so it's quick to demo) */
const int MAX_CAPACITY = 5;

/*            RS  E  D4 D5 D6 D7 */
LiquidCrystal lcd(12, 11, 6, 7, 8, 9);

/* Occupancy information for the facility */
typedef struct {
  int current;
  int maxCapacity;
  unsigned long totalEntries;
  unsigned long totalExits;
  unsigned long rejectedEntries;
  unsigned long rejectedExits;
} OccupancyRecord;

/* Debounce state for one push button */
typedef struct {
  int pin;
  int stableState;
  int lastReading;
  unsigned long lastChangeTime;
} Button;

/* Function prototypes */
OccupancyRecord *createRecord(int maxCapacity);
void releaseRecord(OccupancyRecord **record);
bool buttonPressed(Button *b);
void handleEntry(OccupancyRecord *record);
void handleExit(OccupancyRecord *record);
void updateLeds(const OccupancyRecord *record);
void updateLcd(const OccupancyRecord *record);
void printStatus(const OccupancyRecord *record);
void printStatistics(const OccupancyRecord *record);
void handleSerialCommands(void);

/* Globals */
OccupancyRecord *occupancy = NULL;
Button entryButton = {ENTRY_PIN, HIGH, HIGH, 0};
Button exitButton = {EXIT_PIN, HIGH, HIGH, 0};
unsigned long lastLcdUpdate = 0;

void setup() {
  Serial.begin(9600);
  pinMode(ENTRY_PIN, INPUT_PULLUP);
  pinMode(EXIT_PIN, INPUT_PULLUP);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  lcd.begin(16, 2);

  occupancy = createRecord(MAX_CAPACITY);
  if (occupancy == NULL) {
    Serial.println("[FATAL] Memory allocation failed. System halted.");
    lcd.print("Memory error");
    digitalWrite(RED_LED, HIGH);
    while (true) { }   /* nothing can work without the record */
  }

  Serial.println("=== Smart Occupancy System Ready ===");
  Serial.println("Commands: s = statistics, r = reset");
  printStatus(occupancy);
  updateLeds(occupancy);
  updateLcd(occupancy);
}

void loop() {
  if (occupancy == NULL)
    return;

  if (buttonPressed(&entryButton))
    handleEntry(occupancy);

  if (buttonPressed(&exitButton))
    handleExit(occupancy);

  handleSerialCommands();

  if (millis() - lastLcdUpdate >= LCD_REFRESH_MS) {
    lastLcdUpdate = millis();
    updateLcd(occupancy);
  }
}

/* Allocates and initialises an occupancy record; returns NULL if allocation fails */
OccupancyRecord *createRecord(int maxCapacity) {
  OccupancyRecord *record = (OccupancyRecord *)malloc(sizeof(OccupancyRecord));

  if (record == NULL)
    return NULL;

  record->current = 0;
  record->maxCapacity = maxCapacity;
  record->totalEntries = 0;
  record->totalExits = 0;
  record->rejectedEntries = 0;
  record->rejectedExits = 0;
  return record;
}

/* Frees the record and sets the caller's pointer to NULL */
void releaseRecord(OccupancyRecord **record) {
  free(*record);
  *record = NULL;
}

/* Returns true once per press, after the signal has been stable for DEBOUNCE_MS */
bool buttonPressed(Button *b) {
  int reading = digitalRead(b->pin);
  bool pressed = false;

  if (reading != b->lastReading)
    b->lastChangeTime = millis();

  if (millis() - b->lastChangeTime > DEBOUNCE_MS && reading != b->stableState) {
    b->stableState = reading;
    if (b->stableState == LOW)
      pressed = true;
  }

  b->lastReading = reading;
  return pressed;
}

/* Adds one person, or rejects the entry if the facility is full */
void handleEntry(OccupancyRecord *record) {
  char msg[64];

  if (record->current >= record->maxCapacity) {
    record->rejectedEntries++;
    snprintf(msg, sizeof(msg), "[ERROR] Entry rejected: facility is full (%d/%d).",
             record->current, record->maxCapacity);
    Serial.println(msg);
    updateLeds(record);
    return;
  }

  record->current++;
  record->totalEntries++;
  Serial.print("[ENTRY] Person entered. ");
  printStatus(record);
  updateLeds(record);
}

/* Removes one person, or rejects the exit if the facility is empty */
void handleExit(OccupancyRecord *record) {
  if (record->current <= 0) {
    record->rejectedExits++;
    Serial.println("[ERROR] Exit rejected: facility is already empty.");
    return;
  }

  record->current--;
  record->totalExits++;
  Serial.print("[EXIT] Person left. ");
  printStatus(record);
  updateLeds(record);
}

/* Green when there is space, red when full */
void updateLeds(const OccupancyRecord *record) {
  bool full = record->current >= record->maxCapacity;

  digitalWrite(GREEN_LED, full ? LOW : HIGH);
  digitalWrite(RED_LED, full ? HIGH : LOW);
}

/* Shows occupancy and remaining capacity on the LCD */
void updateLcd(const OccupancyRecord *record) {
  char line[17];
  bool full = record->current >= record->maxCapacity;

  snprintf(line, sizeof(line), "Inside:%3d/%-3d", record->current, record->maxCapacity);
  lcd.setCursor(0, 0);
  lcd.print(line);

  snprintf(line, sizeof(line), "Free:%3d  %s",
           record->maxCapacity - record->current, full ? "FULL" : "OPEN");
  lcd.setCursor(0, 1);
  lcd.print(line);
}

/* Prints current occupancy and available capacity to Serial */
void printStatus(const OccupancyRecord *record) {
  char msg[64];

  snprintf(msg, sizeof(msg), "Inside: %d/%d | Available: %d",
           record->current, record->maxCapacity,
           record->maxCapacity - record->current);
  Serial.println(msg);
}

/* Prints running totals to Serial */
void printStatistics(const OccupancyRecord *record) {
  char msg[64];

  Serial.println("----- Occupancy Statistics -----");
  printStatus(record);
  snprintf(msg, sizeof(msg), "Total entries: %lu | Total exits: %lu",
           record->totalEntries, record->totalExits);
  Serial.println(msg);
  snprintf(msg, sizeof(msg), "Rejected entries: %lu | Rejected exits: %lu",
           record->rejectedEntries, record->rejectedExits);
  Serial.println(msg);
  Serial.println("--------------------------------");
}

/* Reads one-letter commands typed into the Serial Monitor */
void handleSerialCommands(void) {
  char cmd;

  if (Serial.available() == 0)
    return;

  cmd = Serial.read();

  if (cmd == 's' || cmd == 'S') {
    printStatistics(occupancy);
  } else if (cmd == 'r' || cmd == 'R') {
    releaseRecord(&occupancy);
    occupancy = createRecord(MAX_CAPACITY);
    if (occupancy == NULL) {
      Serial.println("[FATAL] Memory allocation failed during reset.");
      digitalWrite(GREEN_LED, LOW);
      digitalWrite(RED_LED, HIGH);
      return;
    }
    Serial.println("[RESET] Record freed and reallocated. Occupancy set to 0.");
    updateLeds(occupancy);
    updateLcd(occupancy);
  }
}