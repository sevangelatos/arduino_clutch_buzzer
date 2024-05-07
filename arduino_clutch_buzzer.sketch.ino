
#define CLUTCH_PIN 3
#define SPEAKER_PIN 8
// In some period (unparking/parking) we allow up to 20 sec of clutch in
#define GRACE_HIGH_MS 20000
// In normal driving 5 sec should be plenty
#define GRACE_LOW_MS 5000

// We will be forgiving if you do not activate the buzzer for 2 minutes
#define BONUS_PERIOD  120000

// We will be forgiving for the first minute after the first clutch in
#define UNPARK_PERIOD 60000

// Possible states
enum FsmState {
  ST_OFF,     // Clutch out
  ST_GRACE,   // Clutch in but within grace period
  ST_BUZZ     // Clutch in and buzzing
};

FsmState state;
// Current grace time allowed without beeping
unsigned grace_time_ms;
// Time when the beeping will start if lutch not released
unsigned long timeout_time;
// Time of first clutch press
unsigned long start_time;
// Time of last activation
unsigned long last_beep_time;

void setup() {
  // Onboard LED mirrors the clutch input
  pinMode(LED_BUILTIN, OUTPUT);
  // We use pin 3 as input
  pinMode(CLUTCH_PIN, INPUT);
  grace_time_ms = GRACE_HIGH_MS;
  state = ST_OFF;
  timeout_time = 0;
  start_time = 0;
  last_beep_time = 0;
}

void buzz(bool on) {
  if (on) {
    tone(SPEAKER_PIN, 1000);
  }
  else {
    noTone(SPEAKER_PIN);
  }
}

bool isClutchIn() {
  // To avoid spurious activations read twice
  const int in1 = digitalRead(CLUTCH_PIN);
  digitalWrite(LED_BUILTIN, in1);
  delay(10);
  const int in2 = digitalRead(CLUTCH_PIN);
  return in1 == HIGH && in2 == HIGH;
}

void loop() {
  bool clutch_in = isClutchIn();
  FsmState new_state = state;
  unsigned long now = millis();

  switch (state) {
    case ST_OFF:
      if (clutch_in) {
        new_state = ST_GRACE;
        timeout_time = now + grace_time_ms;
        // Make a note of the start time
        if (start_time == 0) {
          start_time = now;
        }
      }
      break;

    case ST_GRACE:
      if (clutch_in == false) {
        new_state = ST_OFF;
      } else if (now >= timeout_time) {
        new_state = ST_BUZZ;
      }
      break;

    case ST_BUZZ:
      if (clutch_in == false) {
        new_state = ST_OFF;
      }
      // Next activation will be shorter
      grace_time_ms = GRACE_LOW_MS;
      last_beep_time = now;
      break;
  }

  // If within the unpark period or you got the bonus, keep the grace time high
  if (start_time == 0 ||
      now < start_time + UNPARK_PERIOD ||
      now > last_beep_time + BONUS_PERIOD) {
    grace_time_ms = GRACE_HIGH_MS;
  }

  if (new_state != state) {
    state = new_state;
    buzz(state == ST_BUZZ);
  }
}
