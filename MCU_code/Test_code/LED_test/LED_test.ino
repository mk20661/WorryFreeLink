#define LED_Older_1 6
#define LED_Older_2 7
#define LED_Older_3 8
#define LED_Older_4 9
#define LED_Older_5 10

#define LED_Kid_1 11
#define LED_Kid_2 12
#define LED_Kid_3 13

#define LED_Older_6 0
#define LED_Older_7 1
#define LED_Older_8 2
#define LED_Older_9 3
#define LED_Older_10 4

#define LED_Kid_4 A3
#define LED_Kid_5 A4
#define LED_Kid_6 A5

#define Button 5

void setup() {
    pinMode(LED_Older_1, OUTPUT);
    pinMode(LED_Older_2, OUTPUT);
    pinMode(LED_Older_3, OUTPUT);
    pinMode(LED_Older_4, OUTPUT);
    pinMode(LED_Older_5, OUTPUT);

    pinMode(LED_Kid_1, OUTPUT);
    pinMode(LED_Kid_2, OUTPUT);
    pinMode(LED_Kid_3, OUTPUT);

    pinMode(LED_Older_6, OUTPUT);
    pinMode(LED_Older_7, OUTPUT);
    pinMode(LED_Older_8, OUTPUT);
    pinMode(LED_Older_9, OUTPUT);
    pinMode(LED_Older_10, OUTPUT);

    pinMode(LED_Kid_4, OUTPUT);
    pinMode(LED_Kid_5, OUTPUT);
    pinMode(LED_Kid_6, OUTPUT);

}

void loop() {
    digitalWrite(LED_Older_1, HIGH);
    digitalWrite(LED_Older_2, HIGH);
    digitalWrite(LED_Older_3, HIGH);
    digitalWrite(LED_Older_4, HIGH);
    digitalWrite(LED_Older_5, HIGH);
    digitalWrite(LED_Older_6, HIGH);
    digitalWrite(LED_Older_7, HIGH);
    digitalWrite(LED_Older_8, HIGH);
    digitalWrite(LED_Older_9, HIGH);
    digitalWrite(LED_Older_10, HIGH);

    digitalWrite(LED_Kid_1, LOW);
    digitalWrite(LED_Kid_2, LOW);
    digitalWrite(LED_Kid_3, LOW);
    digitalWrite(LED_Kid_4, LOW);
    digitalWrite(LED_Kid_5, LOW);
    digitalWrite(LED_Kid_6, LOW);


    delay(3000); 

    digitalWrite(LED_Older_1, LOW);
    digitalWrite(LED_Older_2, LOW);
    digitalWrite(LED_Older_3, LOW);
    digitalWrite(LED_Older_4, LOW);
    digitalWrite(LED_Older_5, LOW);
    digitalWrite(LED_Older_6, LOW);
    digitalWrite(LED_Older_7, LOW);
    digitalWrite(LED_Older_8, LOW);
    digitalWrite(LED_Older_9, LOW);
    digitalWrite(LED_Older_10, LOW);

    digitalWrite(LED_Kid_1, HIGH);
    digitalWrite(LED_Kid_2, HIGH);
    digitalWrite(LED_Kid_3, HIGH);
    digitalWrite(LED_Kid_4, HIGH);
    digitalWrite(LED_Kid_5, HIGH);
    digitalWrite(LED_Kid_6, HIGH);

    delay(3000);
}