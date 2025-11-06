/* 
 * File:   main.c
 * Author: E306-PC5
 *
 * Created on October 6, 2025, 9:24 AM
 */

#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include "ChipConfig.h"
#include "IO.h"
#include "timer.h"
#include "PWM.h"
#include "Robot.h"
#include "ADC.h"
#include "main.h"

unsigned char robot_is_running = 0;
unsigned long start_time = 0;
unsigned char stateRobot = STATE_ATTENTE;
unsigned char nextStateRobot = 0;

// Fonction principale
int main(void) {
    // Initialisation
    InitOscillator();
    InitIO();
    InitTimer1();
    InitTimer23();
    InitTimer4();
    InitPWM();
    InitADC1();

    PWMSetSpeedConsigne(0, MOTEUR_DROIT);
    PWMSetSpeedConsigne(0, MOTEUR_GAUCHE);
    PWMUpdateSpeed();

    LED_VERTE_1 = 1;  // Prêt à démarrer
    LED_ROUGE_1 = 0;

    while (1) {
        // Vérification du bouton de démarrage
        if (!robot_is_running && _RH0 == 1) {
            robot_is_running = 1;
            start_time = timestamp; // Enregistre le temps de départ
            stateRobot = STATE_ATTENTE;
            LED_VERTE_1 = 0;
            LED_ROUGE_1 = 1; // Robot en marche
        }

        // Lecture des télémètres
        if (ADCIsConversionFinished()) {
            ADCClearConversionFinishedFlag();
            unsigned int *result = ADCGetResult();

            float volts;

            volts = ((float) result[2])*3.3/4096;
            robotState.distanceTelemetreGauche = 34/volts - 3.3;

            volts = ((float) result[1])*3.3/4096;
            robotState.distanceTelemetreCentre = 34/volts - 3.3;

            volts = ((float) result[3])*3.3/4096;
            robotState.distanceTelemetreDroit = 34/volts - 3.3;

            volts = ((float) result[4])*3.3/4096;
            robotState.distanceTelemetreDroit2 = 34/volts - 3.3;

            volts = ((float) result[0])*3.3/4096;
            robotState.distanceTelemetreGauche2 = 34/volts - 3.3;

            // Mise à jour des LEDs d?obstacle
            if (robot_is_running==1){
            if (robotState.distanceTelemetreCentre >= 30) { 
                LED_ORANGE_1 = 0; } 
            else { LED_ORANGE_1 = 1; } 
            if (robotState.distanceTelemetreGauche >= 30) { 
                LED_BLEUE_1 = 0; } 
            else { LED_BLEUE_1 = 1; } 
            if (robotState.distanceTelemetreGauche2 >= 30) { 
                LED_BLANCHE_1 = 0; } 
            else { LED_BLANCHE_1 = 1; } 
            if (robotState.distanceTelemetreDroit >= 30) { 
                LED_ROUGE_1 = 0; } 
            else { LED_ROUGE_1 = 1; } 
            if (robotState.distanceTelemetreDroit2 >= 30) { 
                LED_VERTE_1 = 0; } 
            else { LED_VERTE_1 = 1; } 
            }
        }

        // Appel de la machine à états
        OperatingSystemLoop();

        // Arrêt automatique après 1 minute
        if (robot_is_running && (timestamp - start_time >= 60000)) {
            robot_is_running = 0;
            PWMSetSpeedConsigne(0, MOTEUR_DROIT);
            PWMSetSpeedConsigne(0, MOTEUR_GAUCHE);
            PWMUpdateSpeed();
            LED_ROUGE_1 = 0;
            LED_VERTE_1 = 1; // prêt à démarrer
        }
    }
}

// Boucle principale de la machine à états
void OperatingSystemLoop(void) {
    if (!robot_is_running) {
        PWMSetSpeedConsigne(0, MOTEUR_DROIT);
        PWMSetSpeedConsigne(0, MOTEUR_GAUCHE);
        return;
    }

    switch (stateRobot) {
        case STATE_ATTENTE:
            PWMSetSpeedConsigne(0, MOTEUR_DROIT);
            PWMSetSpeedConsigne(0, MOTEUR_GAUCHE);
            stateRobot = STATE_ATTENTE_EN_COURS;
            break;

        case STATE_ATTENTE_EN_COURS:
            if (timestamp - start_time > 1000) // 1 seconde
                stateRobot = STATE_AVANCE;
            break;

        case STATE_AVANCE:
            PWMSetSpeedConsigne(30, MOTEUR_DROIT);
            PWMSetSpeedConsigne(30, MOTEUR_GAUCHE);
            stateRobot = STATE_AVANCE_EN_COURS;
            break;

        case STATE_AVANCE_EN_COURS:
        case STATE_TOURNE_GAUCHE_EN_COURS:
        case STATE_TOURNE_DROITE_EN_COURS:
        case STATE_TOURNE_SUR_PLACE_GAUCHE_EN_COURS:
        case STATE_TOURNE_SUR_PLACE_DROITE_EN_COURS:
        case STATE_TOURNE_GAUCHE_GAUCHE_EN_COURS:
        case STATE_TOURNE_DROITE_DROITE_EN_COURS:
            SetNextRobotStateInAutomaticMode();
            break;

        case STATE_TOURNE_GAUCHE:
            PWMSetSpeedConsigne(20, MOTEUR_DROIT);
            PWMSetSpeedConsigne(0, MOTEUR_GAUCHE);
            stateRobot = STATE_TOURNE_GAUCHE_EN_COURS;
            break;

        case STATE_TOURNE_DROITE:
            PWMSetSpeedConsigne(0, MOTEUR_DROIT);
            PWMSetSpeedConsigne(20, MOTEUR_GAUCHE);
            stateRobot = STATE_TOURNE_DROITE_EN_COURS;
            break;

        case STATE_TOURNE_SUR_PLACE_GAUCHE:
            PWMSetSpeedConsigne(10, MOTEUR_DROIT);
            PWMSetSpeedConsigne(-10, MOTEUR_GAUCHE);
            stateRobot = STATE_TOURNE_SUR_PLACE_GAUCHE_EN_COURS;
            break;

        case STATE_TOURNE_SUR_PLACE_DROITE:
            PWMSetSpeedConsigne(-10, MOTEUR_DROIT);
            PWMSetSpeedConsigne(10, MOTEUR_GAUCHE);
            stateRobot = STATE_TOURNE_SUR_PLACE_DROITE_EN_COURS;
            break;

        case STATE_TOURNE_GAUCHE_GAUCHE:
            PWMSetSpeedConsigne(30, MOTEUR_DROIT);
            PWMSetSpeedConsigne(20, MOTEUR_GAUCHE);
            stateRobot = STATE_TOURNE_GAUCHE_GAUCHE_EN_COURS;
            break;

        case STATE_TOURNE_DROITE_DROITE:
            PWMSetSpeedConsigne(20, MOTEUR_DROIT);
            PWMSetSpeedConsigne(30, MOTEUR_GAUCHE);
            stateRobot = STATE_TOURNE_DROITE_DROITE_EN_COURS;
            break;

        default:
            stateRobot = STATE_ATTENTE;
            break;
    }
}

// Détermine l?état suivant du robot
void SetNextRobotStateInAutomaticMode() {
    unsigned char positionObstacle = PAS_D_OBSTACLE;

    if (robotState.distanceTelemetreDroit < 30 &&
        robotState.distanceTelemetreCentre > 20 &&
        robotState.distanceTelemetreGauche > 25 &&
        robotState.distanceTelemetreGauche2 > 25)
        positionObstacle = OBSTACLE_A_DROITE;

    else if (robotState.distanceTelemetreDroit > 30 &&
             robotState.distanceTelemetreCentre > 20 &&
             robotState.distanceTelemetreGauche < 25 &&
             robotState.distanceTelemetreDroit2 > 25)
        positionObstacle = OBSTACLE_A_GAUCHE;

    else if (robotState.distanceTelemetreCentre < 35)
        positionObstacle = OBSTACLE_EN_FACE;

    else if (robotState.distanceTelemetreDroit2 < 25 &&
             robotState.distanceTelemetreDroit > 25 &&
             robotState.distanceTelemetreCentre > 20 &&
             robotState.distanceTelemetreGauche > 25 &&
             robotState.distanceTelemetreGauche2 > 35)
        positionObstacle = OBSTACLE_A_DROITE_DROITE;

    else if (robotState.distanceTelemetreDroit2 > 25 &&
             robotState.distanceTelemetreDroit > 25 &&
             robotState.distanceTelemetreCentre > 20 &&
             robotState.distanceTelemetreGauche > 25 &&
             robotState.distanceTelemetreGauche2 < 35)
        positionObstacle = OBSTACLE_A_GAUCHE_GAUCHE;

    else if (robotState.distanceTelemetreDroit2 < 20 &&
             robotState.distanceTelemetreDroit > 20 &&
             robotState.distanceTelemetreCentre > 20 &&
             robotState.distanceTelemetreGauche > 20 &&
             robotState.distanceTelemetreGauche2 < 20)
        positionObstacle = PAS_D_OBSTACLE;

    else if (robotState.distanceTelemetreDroit > 20 &&
             robotState.distanceTelemetreCentre > 15 &&
             robotState.distanceTelemetreGauche > 20)
        positionObstacle = PAS_D_OBSTACLE;

    // Détermination de l?état suivant
    switch(positionObstacle) {
        case PAS_D_OBSTACLE: nextStateRobot = STATE_AVANCE; break;
        case OBSTACLE_A_DROITE: nextStateRobot = STATE_TOURNE_GAUCHE; break;
        case OBSTACLE_A_GAUCHE: nextStateRobot = STATE_TOURNE_DROITE; break;
        case OBSTACLE_EN_FACE: nextStateRobot = STATE_TOURNE_SUR_PLACE_GAUCHE; break;
        case OBSTACLE_A_DROITE_DROITE: nextStateRobot = STATE_TOURNE_GAUCHE_GAUCHE; break;
        case OBSTACLE_A_GAUCHE_GAUCHE: nextStateRobot = STATE_TOURNE_DROITE_DROITE; break;
    }

    stateRobot = nextStateRobot;
}
