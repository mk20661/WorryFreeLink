#define LED_Older_1 1
#define LED_Older_2 25
#define LED_Older_3 26
#define LED_Older_4 27
#define LED_Older_5 9

#define LED_Kid_1 10
#define LED_Kid_2 13
#define LED_Kid_3 5

#define Button 3

void setup() {
    pinMode(LED_Older_1, OUTPUT);
    pinMode(LED_Older_2, OUTPUT);
    pinMode(LED_Older_3, OUTPUT);
    // pinMode(LED_Older_4, OUTPUT);
    //pinMode(LED_Older_5, OUTPUT);

    //pinMode(LED_Kid_1, OUTPUT);
    // pinMode(LED_Kid_2, OUTPUT);
    // pinMode(LED_Kid_3, OUTPUT);

}

void loop() {
    digitalWrite(LED_Older_1, HIGH);
    digitalWrite(LED_Older_2, HIGH);
    digitalWrite(LED_Older_3, HIGH);
    // digitalWrite(LED_Older_4, HIGH);
    //digitalWrite(LED_Older_5, HIGH);

    //digitalWrite(LED_Kid_1, LOW);
    // digitalWrite(LED_Kid_2, LOW);
    // digitalWrite(LED_Kid_3, LOW);

    delay(3000); 

    digitalWrite(LED_Older_1, LOW);
    digitalWrite(LED_Older_2, LOW);
    digitalWrite(LED_Older_3, LOW);
    //digitalWrite(LED_Older_4, LOW);
    //digitalWrite(LED_Older_5, LOW);

    //digitalWrite(LED_Kid_1, HIGH);
    // digitalWrite(LED_Kid_2, HIGH);
    // digitalWrite(LED_Kid_3, HIGH);

    delay(3000);
}