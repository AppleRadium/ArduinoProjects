
//This is the Arduino code for the Morse Machine
// by Robert Fleming

const int buzzerPin = 9; 
const int laserPin = 7;
const int clkPin = 2;
const int dtPin  = 3;
const int swPin  = 4;

int volume = 128;
int lastClkState;
int speedUnit = 65; 

bool muted = false;
unsigned long lastButtonTime = 0;
const unsigned long debounceDelay = 400;

enum EncoderMode {
  MODE_VOLUME,
  MODE_SPEED
};

EncoderMode encoderMode = MODE_VOLUME;

// letters and numbers
const char letters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

//Morse characters
const char* morse[] = {
  ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---",
  "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.", "...", "-",
  "..-", "...-", ".--", "-..-", "-.--", "--..",
  "-----", ".----", "..---", "...--", "....-", ".....",
  "-....", "--...", "---..", "----."
};

void setup() {
  pinMode(buzzerPin, OUTPUT);
  pinMode(laserPin, OUTPUT);

  pinMode(clkPin, INPUT);
  pinMode(dtPin, INPUT);
  pinMode(swPin, INPUT_PULLUP);

  digitalWrite(buzzerPin, LOW);
  digitalWrite(laserPin, LOW);

  lastClkState = digitalRead(clkPin);

  Serial.begin(9600);
  Serial.println("Enter a message to transmit in Morse code:");
}

void loop() {

  if (Serial.available()) {
    String message = Serial.readStringUntil('\n');
    message.toUpperCase();

    Serial.print("Transmitting: ");
    Serial.println(message);

    transmitMessage(message);

    Serial.println("Done. Enter another message:");
  }

}

void readEncoder() {
  int clkState = digitalRead(clkPin);

  if (clkState != lastClkState) {
    int direction = (digitalRead(dtPin) != clkState) ? 1 : -1;

    if (encoderMode == MODE_VOLUME) {
      volume += direction * 5;
      volume = constrain(volume, 0, 255);

      Serial.print("Volume: ");
      Serial.println(volume);
    }
    else if (encoderMode == MODE_SPEED) {
      speedUnit += direction * 5;
      speedUnit = constrain(speedUnit, 20, 300);

      Serial.print("Speed unit: ");
      Serial.println(speedUnit);
    }
  }

  lastClkState = clkState;
}


void smartDelay(unsigned long duration) {
  unsigned long start = millis();
  while (millis() - start < duration) {
    readEncoder();   // keep updating volume
    handleModeButton();
  }
}

void transmitMessage(String message) {
  for (int i = 0; i < message.length(); i++) {
    char c = message[i];
    if (c == ' ') {
      smartDelay(speedUnit * 7); // word gap
    } else {
      const char* code = getMorse(c);
      if (code != NULL) {
        transmitMorse(code);
        smartDelay(speedUnit * 3); // letter gap
      }
    }
  }
}

const char* getMorse(char c) {
  for (int i = 0; i < sizeof(letters) - 1; i++) {
    if (letters[i] == c) {
      return morse[i];
    }
  }
  return NULL; // unsupported character
}

void transmitMorse(const char* code) {
  for (int i = 0; code[i] != '\0'; i++) {

    if (!muted) {
      analogWrite(buzzerPin, volume);
    }
    digitalWrite(laserPin, HIGH);

    if (code[i] == '.') {
      smartDelay(speedUnit);
    } else if (code[i] == '-') {
      smartDelay(speedUnit * 3);
    }

    analogWrite(buzzerPin, 0);
    digitalWrite(laserPin, LOW);

    smartDelay(speedUnit);
  }
}

void handleModeButton() {
  static bool lastButtonState = HIGH;
  bool buttonState = digitalRead(swPin);

  if (buttonState == LOW && lastButtonState == HIGH) {
    if (millis() - lastButtonTime > debounceDelay) {
      // Toggle mode
      encoderMode = (encoderMode == MODE_VOLUME)
                    ? MODE_SPEED
                    : MODE_VOLUME;

      Serial.print("Encoder mode: ");
      Serial.println(encoderMode == MODE_VOLUME ? "VOLUME" : "SPEED");

      lastButtonTime = millis();
    }
  }

  lastButtonState = buttonState;
}


