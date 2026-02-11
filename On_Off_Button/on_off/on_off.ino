int buttonPin = 14;
bool buttonState = false;
bool powerMode = false;
int loopDelay = 100;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:

  if(buttonState){
    if(digitalRead(buttonPin) == LOW){
      // Button Released
      powerMode = !powerMode;
    }
  }

  if(!buttonState){
    if(digitalRead(buttonPin) == HIGH){
      // Button Pressed
    }
  }

  if(digitalRead(buttonPin) == HIGH){
    buttonState = true;
    Serial.println("button down");
  }
  if(digitalRead(buttonPin) == LOW){
    buttonState = false;
    Serial.println("button up");
  }

  if(powerMode){
    Serial.println("power on");
  }else{
    Serial.println("power off");
  }

  delay(loopDelay);
}
