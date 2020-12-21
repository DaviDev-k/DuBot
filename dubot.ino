/*
 *    ___       ___      _
 *   |   \ _  _| _ ) ___| |_
 *   | |) | || | _ \/ _ \  _|
 *   |___/ \___|___/\___/\__|
 *
 */


#include <SoftwareSerial.h>


// Pin
const int PIN_MOTOR_L1 = 2;
const int PIN_MOTOR_L2 = 3;
const int PIN_MOTOR_R1 = 4;
const int PIN_MOTOR_R2 = 5;
const int PIN_BT_TX = 12;
const int PIN_BT_RX = 13;


// Direzioni
enum DirFB {
    FORTH, BACK
};
enum DirLR {
    LEFT, RIGHT
};



// Classe che specifica i pin e gli stati di un motore
class Motor {

    // Pin di input
    int in1, in2;

private:

    // Setta i pin ai valori specificati
    void bridgeH(uint8_t val1, uint8_t val2);

public:

    // Costruttore - Inizializza il motore
    Motor(int i1, int i2);

    // Movimento
    void move(DirFB dir);

    // Fermo
    void stop();

};


// Costruttore - Inizializza il motore
Motor::Motor(int i1, int i2) {
    in1 = i1;
    in2 = i2;
    pinMode(in1, OUTPUT);
    pinMode(in2, OUTPUT);
    stop();
}

// Setta i pin ai valori specificati
void Motor::bridgeH(uint8_t val1, uint8_t val2) {
    digitalWrite(in1, val1);
    digitalWrite(in2, val2);
}

// Movimento
void Motor::move(DirFB dir) {
    if (dir == FORTH)
        bridgeH(HIGH, LOW);
    if (dir == BACK)
        bridgeH(LOW, HIGH);
}

// Fermo
void Motor::stop() {
    bridgeH(LOW, LOW);
}



// Classe dell'intero robottino che comprende tutti i componenti
class DuBot {

    // Motori sinistro e destro
    Motor mL = Motor(PIN_MOTOR_L1, PIN_MOTOR_L2);
    Motor mR = Motor(PIN_MOTOR_R1, PIN_MOTOR_R2);

public:

    // Costruttore - Inizializza DuBot e i suoi componenti
    DuBot();

    // Movimento in avanti o indietro
    void move(DirFB dir);

    // Fermo
    void stop();

    // Gira a sinistra o destra (stretta)
    void rotate(DirLR dir);

    // Curva a sinistra o destra (larga)
    void curve(DirLR dir);

    // Esecuzione casuale
    void dance(int times);

};

// Costruttore - Ferma DuBot
DuBot::DuBot() {
    stop();
}

// Movimento in avanti o indietro
void DuBot::move(DirFB dir) {
    mL.move(dir);
    mR.move(dir);
}

// Fermo
void DuBot::stop() {
    mL.stop();
    mR.stop();
}

// Ruota a sinistra o destra (stretta)
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
        mR.stop();
        mL.move(FORTH);
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
SoftwareSerial bt(PIN_BT_RX, PIN_BT_TX);


void setup() {
    bt.begin(9600);
    bt.println("Connesso a DuBot");
}


void loop() {

    String btRead = bt.readString(); // legge il seriale bt

    switch (btRead[0]) { // primo carattere
        case 'F':  // forth
            bot.move(FORTH);
            break;
        case 'B':  // back
            bot.move(BACK);
            break;
        case 'l':  // gira a sinistra
            bot.rotate(LEFT);
            break;
        case 'r':  // gira a destra
            bot.rotate(RIGHT);
            break;
        case 'L':  // curva a sinistra
            bot.curve(LEFT);
            break;
        case 'R':  // curva a destra
            bot.curve(RIGHT);
            break;
        case 'S':  // stop
            bot.stop();
    }

    delay(2000);

}
