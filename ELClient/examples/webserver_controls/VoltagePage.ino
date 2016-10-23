#include <ELClientWebServer.h>
#include <avr/io.h>

#define SAMPLE_COUNT 100
#define PERIOD_COUNT (135 * SAMPLE_COUNT)

uint16_t adc_min = 0xFFFF;         // min value of ADC
uint16_t adc_max = 0;              // max value of ADC
uint32_t adc_avg = 0;              // AVG average

uint16_t sample_count;             // collected sample
uint32_t voltage_avg = 0;          // AVG used for voltage measurement
uint16_t measured_voltage = 0;     // measured voltage

#define MAX_HISTORY 3              // max history count

uint8_t  history_cnt = 0;          // number of histories
uint32_t history_ts[MAX_HISTORY];  // timestamp
uint16_t history_min[MAX_HISTORY]; // min
uint16_t history_max[MAX_HISTORY]; // max
uint16_t history_avg[MAX_HISTORY]; // avg

uint16_t calibrate = 0x128; // calibrate this manually

// voltage measuring loop
void voltageLoop()
{
  uint16_t adc = analogRead(A0); // read ADC

  // set min/max/avg
  if( adc < adc_min )
    adc_min = adc;
  if( adc > adc_max )
    adc_max = adc;
  adc_avg += adc;
  
  voltage_avg += adc;
  sample_count++;

  if( (sample_count % SAMPLE_COUNT) == 0 ) // max samples reached?
  {
    // calculate measured voltage
    voltage_avg /= SAMPLE_COUNT;
    measured_voltage = voltage_avg * calibrate / 256;
    voltage_avg = 0;
  }
  if( sample_count == PERIOD_COUNT )
  {
    // calculate min/max/avg and put into history
    
    for(int8_t i=MAX_HISTORY-2; i >=0; i-- )
    {
      history_ts[i+1] = history_ts[i];
      history_min[i+1] = history_min[i];
      history_max[i+1] = history_max[i];
      history_avg[i+1] = history_avg[i];
    }

    history_ts[0] = millis();
    history_min[0] = (uint32_t)adc_min * calibrate / 256;
    history_max[0] = (uint32_t)adc_max * calibrate / 256;
    history_avg[0] = (adc_avg / PERIOD_COUNT) * calibrate / 256;

    adc_min = 0xFFFF;
    adc_max = 0;
    adc_avg = 0;

    if( history_cnt < MAX_HISTORY )
      history_cnt++;
    sample_count = 0;
  }
}

// sprintf %f is not supported on Arduino...
String floatToString(float f)
{
  int16_t intg = (int16_t)(f * 100.f);
  int16_t int_part = intg / 100;
  int16_t fract_part = intg % 100;

  char buf[20];
  sprintf(buf, "%d.%02d", int_part, fract_part);
  
  return String(buf);
}

void voltageRefreshCb(const char * url)
{
  // calculate voltage value
  String v = floatToString((float)measured_voltage / 256.f);
  v += " V";
  webServer.setArgString(F("voltage"), v.begin());

  char buf[20];
  // calculate history table
  String table = F("[[\"Time\",\"Min\",\"AVG\",\"Max\"]");

  for(uint8_t i=0; i < history_cnt; i++ )
  {
    float min_f = (float)history_min[i] / 256.f;
    float max_f = (float)history_max[i] / 256.f;
    float avg_f = (float)history_avg[i] / 256.f;

    table += F(",[\"");
    table += (history_ts[i] / 1000);
    table += F(" s\",\"");
    table += floatToString(min_f);
    table += " V\",\"";
    table += floatToString(avg_f);
    table += " V\",\"";
    table += floatToString(max_f);
    table += " V\"]";
  }

  table += ']';
  webServer.setArgJson(F("table"), table.begin());
}

// page setup
void voltageInit()
{
  analogReference(DEFAULT);
  sample_count = 0;
  
  URLHandler *voltageHandler = webServer.createURLHandler(F("/Voltage.html.json"));

  voltageHandler->loadCb.attach(voltageRefreshCb);
  voltageHandler->refreshCb.attach(voltageRefreshCb);
}
