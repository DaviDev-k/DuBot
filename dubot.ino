/*
 *    ___       ___      _
 *   |   \ _  _| _ ) ___| |_
 *   | |) | || | _ \/ _ \  _|
 *   |___/ \___|___/\___/\__|
 *
 */


#include <SoftwareSerial.h>


// Pin
#define PIN_WHEEL_L1 13
#define PIN_WHEEL_L2 12
#define PIN_WHEEL_R1 9
#define PIN_WHEEL_R2 4

#define PIN_CHEST_1 8
#define PIN_CHEST_2 7
#define PIN_ARMS_1 11
#define PIN_ARMS_2 10

#define PIN_US_TRIG 2
#define PIN_US_ECHO 3

#define PIN_BT_RX 6
#define PIN_BT_TX 5


// Direzioni
enum dirFB {
    FORTH, BACK
};
enum dirLR {
    LEFT, RIGHT
};
enum chestUD {
    UP, DOWN
};
enum armsOC {
    OPEN, CLOSE
};


/**************** MOTOR ****************/
// Classe che specifica i pin e gli stati di un motore
class Motor {

    // Pin di output
    int out1, out2;

    // Setta i pin ai valori specificati
    void setBridgeH(uint8_t val1, uint8_t val2);

public:

    // Costruttore
    Motor(int o1, int o2);

    // Movimento
    void move(bool dir);

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
void Motor::setBridgeH(uint8_t val1, uint8_t val2) {
    digitalWrite(out1, val1);
    digitalWrite(out2, val2);
}

// Movimento avanti o indietro
void Motor::move(bool dir) {
    if (dir == FORTH)
        setBridgeH(HIGH, LOW);
    if (dir == BACK)
        setBridgeH(LOW, HIGH);
}

// Ferma il motore
void Motor::stop() {
    setBridgeH(LOW, LOW);
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
    Motor wheelL;
    Motor wheelR;
    Motor chest;
    Motor arms;

public:

    // Evita gli ostacoli
    bool useUltra;
    // Stato
    bool isMoving;
    // Bluethoot
    SoftwareSerial bt;
    // Ultrasuoni
    Ultra ultra;

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
    void runBTcommand(char cmd);

};

// Costruttore - Ferma DuBot
DuBot::DuBot() : wheelL(PIN_WHEEL_L1, PIN_WHEEL_L2),
                 wheelR(PIN_WHEEL_R1, PIN_WHEEL_R2),
                 chest(PIN_CHEST_1, PIN_CHEST_2),
                 arms(PIN_ARMS_1, PIN_ARMS_2),
                 bt(PIN_BT_TX, PIN_BT_RX) {
    bt.begin(9600);
    bt.println("Connesso a DuBot");
    stop();
    arms.stop();
    chest.stop();
    useUltra = true;
}

// Movimento in avanti o indietro
void DuBot::move(dirFB dir) {
    isMoving = (dir == FORTH);
    wheelL.move(dir);
    wheelR.move(dir);
}

// Ferma DuBot
void DuBot::stop() {
    isMoving = false;
    wheelL.stop();
    wheelR.stop();
}

// Gira a sinistra o destra (stretta)
void DuBot::rotate(dirLR dir) {
    isMoving = false;
    wheelL.move(dir == LEFT ? BACK : FORTH);
    wheelR.move(dir == RIGHT ? BACK : FORTH);
}

// Curva a sinistra o destra (larga)
void DuBot::curve(dirLR dir) {
    isMoving = true;
    if (dir == LEFT) {
        wheelL.stop();
        wheelR.move(FORTH);
    }
    if (dir == RIGHT) {
        wheelL.move(FORTH);
        wheelR.stop();
    }
}

// Esecuzione casuale
void DuBot::dance(int times) {
    isMoving = true;
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

        int distance = ultra.distance();
        bt.print("cm ");
        bt.println(distance);

        obstacle = (distance < DIST);
        if (obstacle) {
            rotate(LEFT);
            loop = true;
        }

    } while (obstacle);

    // Va avanti solo dopo aver evitato un ostacolo, altrimenti non cambia marcia
    if (loop)
        move(FORTH);

}

// Esegue i comandi inviati dal bluetooth
void DuBot::runBTcommand(char cmd) {

    switch (cmd) {  // Comando bluetooth

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

        case 'U':  // Alza il corpo
            chest.move(UP);
            delay(400);
            chest.stop();
            break;
        case 'D':   // Abbassa il corpo
            chest.move(DOWN);
            delay(60);
            chest.stop();
            break;

        case 'O':   // Apre le braccia
            arms.move(OPEN);
            delay(600);
            arms.stop();
            break;
        case 'C':   // Chiude le braccia
            arms.move(CLOSE);
            delay(600);
            arms.stop();
            break;

        case 'S':  // Stop
            stop();
            break;

        case 'd':  // Dance
            dance(8);
            break;

        case 'u':  // Usa o no ultrasuoni
            useUltra = !useUltra;
            break;

    }

}


/**************** ARDUINO ****************/

DuBot bot;

void setup() {}

void loop() {
    if (bot.useUltra && bot.isMoving)
        bot.escapeObstacle(24);
    bot.runBTcommand(bot.bt.read());
    delay(10);
}
