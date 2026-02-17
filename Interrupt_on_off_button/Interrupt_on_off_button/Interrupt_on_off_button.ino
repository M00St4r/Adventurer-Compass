bool powerState = false;
uint32_t ISR_millis= 0;

struct Button {
  const uint8_t PIN;
  volatile uint32_t numberKeyPresses;
  volatile bool pressed;
};

Button button1 = { 26, 0, false };

void IRAM_ATTR isr(){
  // use timer to holdoff for 100mS after each count
  if( (millis() - ISR_millis) > 150 ){
    ISR_millis = millis();
    Serial.println("power Button Pressed");
    powerState = !powerState;
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(button1.PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(button1.PIN), isr, FALLING);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(powerState){
    Serial.println("power on");
  }else{
    Serial.println("power off");
  }
  // Serial.print("pin val: ");
  // Serial.println(analogRead(buttonPin));
  delay(500);
}
