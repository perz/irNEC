

#define IRPIN PB5
volatile unsigned long irTmr;
volatile unsigned long irCommand;

volatile boolean waitForStart = false;
volatile boolean startSet = false;
volatile boolean gotIr = false;
volatile boolean repeatIr = false;
volatile byte irBits, irAdr,  irData;
#define BOARD_LED_PIN 33
#define LED_PIN PC13
#define NECREPEAT 8000
#define NECSTARTHIGH  14000
#define NECSTARTLOW 4000
#define NECONE 1688
#define NECZERO 800
#define NECONEHIGH 2812
void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(IRPIN, INPUT_PULLUP);
  //   attachInterrupt(digitalPinToInterrupt(IRPIN), irRead, FALLING);  // Nano
  attachInterrupt(IRPIN, irRead, FALLING);
  Serial.begin(115200);
  Serial.println("Setup done");
}

void loop() {
  if (gotIr) {
    Serial.print("Ir data= ");
    Serial.println(irData, HEX);
    Serial.print("Ir adress= ");
    Serial.println(irAdr, HEX);
    gotIr = false;
    digitalWrite(LED_PIN, LOW);
  }
  if (repeatIr) {
    Serial.print("Ir repeat= ");

    Serial.println(irData, HEX);
    repeatIr = false;
    digitalWrite(LED_PIN, HIGH);
  }
  if (irData == 2)
    digitalWrite(LED_PIN, LOW);
  delay(50);
  // Serial.println("Wait for ircomand");
  if (irData == 0xB0)
    digitalWrite(LED_PIN, HIGH);
  delay(100);

}
/*  Read NEC ir interrupt function  
Use: irTmr 4bytes
    irCommand 4 bytes
    irBits   1 byte
    irData 1 byte   Some knob has been pushed :-) on the remote if gotIr is true read it and handle (check if its the right adress maybe) after handled set gotIr to false 
    irAdr 1 byte  The remote has this adress
    and four boolean
    gotIr =  a ircommand is waiting to be handled
    repeatIr = a ircommand AND a repeatsignal has been sent
*****************************************/
void irRead() {
  unsigned long timez;
  timez = micros() - irTmr;
  irTmr = micros();

  if (startSet) {
    if (timez > NECONEHIGH) {
      // Serial.print("NEC one high> ");
      //Serial.print(timez);
      // Serial.print(" ");
      // Serial.println(irBits);
      if (timez > NECREPEAT)
        repeatIr = true;
      startSet = false;
      irBits = 0;

    }
    else if (timez > NECONE) {

      bitSet(irCommand, (31 - irBits));
      irBits++;
      //Serial.print("1 ");

    }
    else if (timez > NECZERO) {
      bitClear(irCommand, (31 - irBits));
      irBits++;
      // Serial.print("0 ");
    }
    else {
      startSet = false;
      irBits = 0;
      //Serial.print("Nec command error to short");
    }
  }
  else if (timez > NECSTARTHIGH) {
    waitForStart = true;
    //Serial.println(">");
    //Serial.print("Nec wait for start");
    //Serial.println(timez);
  }
  else if (timez > NECSTARTLOW) {
    startSet = true;
    irBits = 0;
  }

  if (irBits == 32) {
    startSet = false;
    irBits = 0;
    gotIr = true;
    irData = (byte)(( irCommand >> 8) & 0xFF);
    //irDataN = (byte)(0xff & irCommand);
    irAdr = (byte)((irCommand >> 24) & 0xFF);


  }

}
