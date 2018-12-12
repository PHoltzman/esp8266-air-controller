#define LEFT_PIN 0
#define RIGHT_PIN 2
#define led LED_BUILTIN

void setup() {
  pinMode(LEFT_PIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  pinMode(RIGHT_PIN, OUTPUT);
  pinMode(led, OUTPUT);
  digitalWrite(LEFT_PIN, LOW);
  digitalWrite(RIGHT_PIN, LOW);
  delay(1000);
}

/*
  Enable a pin (presumably connected to a solenoid) for a provided duration (milliseconds)
*/
void pulse(int pin, int dur) {
  digitalWrite(pin, HIGH);
  digitalWrite(led, LOW);
  delay(dur);
  digitalWrite(pin, LOW);
  digitalWrite(led, HIGH);
}

// the loop function runs over and over again forever
void loop() {
  pulse(LEFT_PIN, 1000);
//  pulse(RIGHT_PIN, 1000);
  delay(150000);
//  pulse(RIGHT_PIN, 1000);
  delay(150000);
//  pulse(LEFT_PIN, 1000);
//  pulse(RIGHT_PIN, 1000);
  delay(150000);
//  pulse(RIGHT_PIN, 1000);
  delay(150000);
}
