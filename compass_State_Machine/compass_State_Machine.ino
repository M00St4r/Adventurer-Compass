enum {IDLE, SEARCH, QUEST}
unsigned char compassState = IDLE;

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  switch (compassState){
    case IDLE:
      if (/*power Button has been pressed*/){
        compassState = SEARCH;
      }else{
        break;
      }
    case SEARCH:
      if (/*power Button has been pressed*/){
        compassState = IDLE;
      }
      //do search
      if(/*found station*/){
        compassState = QUEST;
      }
    case QUEST:
      if (/*power Button has been pressed*/){
        compassState = IDLE;
      }
      //do quest stuff

  }

}
