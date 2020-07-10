/******************************************
 * ESP32 pin assignments                  *
 *                                        */
/* INPUT PINS                             */
#define DHT_SENSOR_PIN       21
#define MOTION_SENSOR_PIN    12
#define LIGHT_SENSOR_PIN     36
#define GAS_SENSOR_PIN       26
#define SOUND_SENSOR_PIN      4
#define CONFIG_BUTTON        23
#define DELETE_CONFIG_BUTTON 22

/* SPI PINS                               */
#define rst                  15
#define dc                   19
#define mosi                 18
#define sclk                  5
#define cs                   32

/* OUTPUT PINS                            */
#define STATUS_LED           13
#define NEO_STATUS_LED_PIN   14
#define NEO_NIGHTLIGHT_PIN   27
#define BUZZER_PIN           25
#define TFT_RS               33
/******************************************/