
#define CLUTCH_PIN 3
#define SPEAKER_PIN 8
#define GRACE_INITIAL_MS 8000
#define GRACE_SUBSEQUENT_MS 5000


// Possible states
enum FsmState {
  ST_OFF,     // Clutch out
  ST_GRACE,   // Clutch in but within grace period
  ST_BUZZ     // Clutch in and buzzing
};

FsmState state;
unsigned grace_time_ms;
unsigned long timeout_time;

void setup() {
  // Onboard LED mirrors the clutch input
  pinMode(LED_BUILTIN, OUTPUT);
  // We use pin 3 as input
  pinMode(CLUTCH_PIN, INPUT);
  grace_time_ms = GRACE_INITIAL_MS;
  state = ST_OFF;
  timeout_time = 0;
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

  switch (state) {
    case ST_OFF:
      if (clutch_in) {
        new_state = ST_GRACE;
        timeout_time = millis() + grace_time_ms;
      }
      break;

    case ST_GRACE:
      if (clutch_in == false) {
        new_state = ST_OFF;
      } else if (millis() >= timeout_time) {
        new_state = ST_BUZZ;
      }
      break;

    case ST_BUZZ:
      if (clutch_in == false) {
        new_state = ST_OFF;
      }
      // Next activation will be shorter
      grace_time_ms = GRACE_SUBSEQUENT_MS;
      break;
  }

  if (new_state != state) {
    state = new_state;
    buzz(state == ST_BUZZ);
  }
}
