#ifndef TIMER_H
#define TIMER_H
void InitTimer23(void);
void InitTimer1(void);
void InitTimer4(void);
extern unsigned long timestamp;
void SetFreqTimer1(float);
void SetFreqTimer4(float);
#endif /* TIMER_H */