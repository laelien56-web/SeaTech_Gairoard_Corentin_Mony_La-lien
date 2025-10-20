
#ifndef PWM_H
#define	PWM_H

#define MOTEUR_DROIT 1
#define MOTEUR_GAUCHE 2
void InitPWM(void);
void PWMSetSpeedConsigne(float vitesseEnPourcents,char moteur);
void PWMUpdateSpeed();

#endif	/* PWM_H */

