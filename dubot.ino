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
#define PIN_US_TRIG 9
#define PIN_US_ECHO 10
#define PIN_BT_RX 12
#define PIN_BT_TX 13


// Direzioni
enum dirFB {
    FORTH, BACK
};
enum dirLR {
    LEFT, RIGHT
};


/**************** MOTOR ****************/
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
    void move(dirFB dir);

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
void Motor::move(dirFB dir) {
    if (dir == FORTH)
        bridgeH(HIGH, LOW);
    if (dir == BACK)
        bridgeH(LOW, HIGH);
}

// Ferma il motore
void Motor::stop() {
    bridgeH(LOW, LOW);
}


/**************** ULTRA ****************/
// Ultrasuoni
class Ultra {

public:

    // Costruttore - Setta i pin
    Ultra();

    // Calcola la distanza dell'ostacolo
    int distance();

};

// Costruttore - Setta i pin
Ultra::Ultra() {
    pinMode(PIN_US_TRIG, OUTPUT);
    pinMode(PIN_US_ECHO, INPUT);
    digitalWrite(PIN_US_TRIG, LOW);
    digitalWrite(PIN_US_ECHO, LOW);
}

int Ultra::distance() {
    // Invia un impulso di 10 μs sul pin trigger
    digitalWrite(PIN_US_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN_US_TRIG, LOW);

    // Riceve metà del tempo in μs per i quali il pin echo e' rimasto allo stato HIGH
    int time = pulseIn(PIN_US_ECHO, HIGH) / 2;

    // La velocita' del suono e' di 340 metri al secondo (340 m/s = 0.00340 cm/μs)
    return time * 3.40e-2;
}


/**************** DUBOT ****************/
// Classe dell'intero robottino che comprende tutti i componenti
class DuBot {

    // Motori sinistro e destro
    Motor mL;
    Motor mR;

public:

    // Stato
    bool moving;
    // Bluethoot
    SoftwareSerial bt;
    // Ultrasuoni
    Ultra us;

public:

    // Costruttore - Inizializza DuBot e i suoi componenti
    DuBot();

    // Movimento in avanti o indietro
    void move(dirFB dir);

    // Gira a sinistra o destra
    void rotate(dirLR dir);

    // Curva a sinistra o destra
    void curve(dirLR dir);

    // Fermo
    void stop();

    // Esecuzione casuale
    void dance(int times);

    // Evita gli ostacoli
    void escapeObstacle(const int DIST);

    // Esegue i comandi inviati dal bluetooth
    void runBTcommand();

};

// Costruttore - Ferma DuBot
DuBot::DuBot() : mL(PIN_MOTOR_L1, PIN_MOTOR_L2),
                 mR(PIN_MOTOR_R1, PIN_MOTOR_R2),
                 bt(PIN_BT_TX, PIN_BT_RX) {
    bt.begin(9600);
    bt.println("Connesso a DuBot");
    stop();
}

// Movimento in avanti o indietro
void DuBot::move(dirFB dir) {
    moving = true;
    mL.move(dir);
    mR.move(dir);
}

// Ferma DuBot
void DuBot::stop() {
    moving = false;
    mL.stop();
    mR.stop();
}

// Gira a sinistra o destra (stretta)
void DuBot::rotate(dirLR dir) {
    moving = false;
    mL.move(dir == LEFT ? BACK : FORTH);
    mR.move(dir == RIGHT ? BACK : FORTH);
}

// Curva a sinistra o destra (larga)
void DuBot::curve(dirLR dir) {
    moving = true;
    if (dir == LEFT) {
        mL.stop();
        mR.move(FORTH);
    }
    if (dir == RIGHT) {
        mL.move(FORTH);
        mR.stop();
    }
}

// Esecuzione casuale
void DuBot::dance(int times) {
    moving = true;
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

// Sfugge agli ostacoli più vicini della distanza specificata
void DuBot::escapeObstacle(const int DIST) {

    bool obstacle;      // true fintanto che e' presente un ostacolo
    bool loop = false;  // true quando è entrato in loop per evitare un ostacolo

    // Ruota a sinista se è presente un oggetto entro una distanza specificata
    do {

        int distance = us.distance();
        bt.println(distance);
        bt.println(" cm");

        obstacle = distance < DIST;
        if (obstacle) {
            rotate(LEFT);
            loop = true;
        }

    } while (obstacle);

    // Va avanti solo dopo aver evitato un ostacolo, altrimenti non cambia marcia
    if (loop)
        move(FORTH);

}

void DuBot::runBTcommand() {

    switch (bt.read()) {  // Comando bluetooth
        case 'F':  // Avanti
            move(FORTH);
            break;
        case 'B':  // Indietro
            move(BACK);
            break;
        case 'L':  // Gira a sinistra
            rotate(LEFT);
            break;
        case 'R':  // Gira a destra
            rotate(RIGHT);
            break;
        case 'l':  // Curva a sinistra
            curve(LEFT);
            break;
        case 'r':  // Curva a destra
            curve(RIGHT);
            break;
        case 'S':  // Stop
            stop();
            break;
        case 'D':  // Dance
            dance(8);
            break;
    }

}


/**************** ARDUINO ****************/

DuBot bot;

void setup() {}

void loop() {
    if (bot.moving)
        bot.escapeObstacle(30);
    bot.runBTcommand();
    delay(10);
}
