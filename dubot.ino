/*
 *    ___       ___      _
 *   |   \ _  _| _ ) ___| |_
 *   | |) | || | _ \/ _ \  _|
 *   |___/ \___|___/\___/\__|
 *
 */


#include <SoftwareSerial.h>


// Pin
#define PIN_MOTOR_L1 3
#define PIN_MOTOR_L2 8
#define PIN_MOTOR_R1 2
#define PIN_MOTOR_R2 11
#define PIN_BT_RX 12
#define PIN_BT_TX 13


// Direzioni
enum DirFB {
    FORTH, BACK
};
enum DirLR {
    LEFT, RIGHT
};


// Classe che specifica i pin e gli stati di un motore
class Motor {

    // Pin di output
    int out1, out2;

private:

    // Setta i pin ai valori specificati
    void bridgeH(uint8_t val1, uint8_t val2);

public:

    // Costruttore
    Motor(int o1, int o2);

    // Movimento
    void move(DirFB dir);

    // Fermo
    void stop();

};

// Costruttore - Inizializza i pin e il motore
Motor::Motor(int o1, int o2) {
    out1 = o1;
    out2 = o2;
    pinMode(out1, OUTPUT);
    pinMode(out2, OUTPUT);
    stop();
}

// Setta i pin ai valori specificati
void Motor::bridgeH(uint8_t val1, uint8_t val2) {
    digitalWrite(out1, val1);
    digitalWrite(out2, val2);
}

// Movimento avanti o indietro
void Motor::move(DirFB dir) {
    if (dir == FORTH)
        bridgeH(HIGH, LOW);
    if (dir == BACK)
        bridgeH(LOW, HIGH);
}

// Ferma il motore
void Motor::stop() {
    bridgeH(LOW, LOW);
}


// Classe dell'intero robottino che comprende tutti i componenti
class DuBot {

    // Motori sinistro e destro
    Motor mL;
    Motor mR;

public:

    // Costruttore - Inizializza DuBot e i suoi componenti
    DuBot();

    // Movimento in avanti o indietro
    void move(DirFB dir);

    // Gira a sinistra o destra
    void rotate(DirLR dir);

    // Curva a sinistra o destra
    void curve(DirLR dir);

    // Fermo
    void stop();

    // Esecuzione casuale
    void dance(int times);

};

// Costruttore - Ferma DuBot
DuBot::DuBot() : mL(PIN_MOTOR_L1, PIN_MOTOR_L2),
                 mR(PIN_MOTOR_R1, PIN_MOTOR_R2) {
    stop();
}

// Movimento in avanti o indietro
void DuBot::move(DirFB dir) {
    mL.move(dir);
    mR.move(dir);
}

// Ferma DuBot
void DuBot::stop() {
    mL.stop();
    mR.stop();
}

// Gira a sinistra o destra (stretta)
void DuBot::rotate(DirLR dir) {
    mL.move(dir == LEFT ? BACK : FORTH);
    mR.move(dir == RIGHT ? BACK : FORTH);
}

// Curva a sinistra o destra (larga)
void DuBot::curve(DirLR dir) {
    if (dir == LEFT) {
        mL.stop();
        mR.move(FORTH);
    }
    if (dir == RIGHT) {
        mL.move(FORTH);
        mR.stop();
    }
}

void DuBot::dance(int times) {
    for (int i = 0; i < times; i++) {
        int dir = random(0, 2);
        switch (random(0, 3)) {
            case 0:
                move(dir);
                break;
            case 1:
                curve(dir);
                break;
            case 2:
                rotate(dir);
                break;
        }
        delay(random(500, 2000));
    }
}


DuBot bot;

// Bluethoot
SoftwareSerial bt(PIN_BT_TX, PIN_BT_RX);


void setup() {
    bt.begin(9600);
    bt.println("Connesso a DuBot");
}


void loop() {

    switch (bt.read()) {  // Comando bluetooth
        case 'F':  // Avanti
            bot.move(FORTH);
            break;
        case 'B':  // Indietro
            bot.move(BACK);
            break;
        case 'L':  // Gira a sinistra
            bot.rotate(LEFT);
            break;
        case 'R':  // Gira a destra
            bot.rotate(RIGHT);
            break;
        case 'l':  // Curva a sinistra
            bot.curve(LEFT);
            break;
        case 'r':  // Curva a destra
            bot.curve(RIGHT);
            break;
        case 'S':  // Stop
            bot.stop();
            break;
        case 'D':  // Dance
            bot.dance(8);
            break;
    }

    delay(10);

}
