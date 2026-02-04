#ifndef LED_ALERTS_H
#define LED_ALERTS_H

void led_alerts_init(void);
void led_alert_success(void);
void led_alert_capture(void);
void led_alert_error(void);
void led_pulse(int duration_ms);

#endif
