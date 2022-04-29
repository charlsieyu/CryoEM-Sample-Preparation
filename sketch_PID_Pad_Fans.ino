#include <Adafruit_MLX90614.h>
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

#define HEATERPIN 6     // Heating Pad Pin
#define FANPIN 9        // Fan Pin

// PID Parameters
double setPoint_temp = 40;
#define Kp_temp      8
#define Ki_temp      0
#define Kd_temp      90

#define i_error_max_temp  4
#define d_error_max_temp  4
int PID_Output_max_temp = (5 * Kp_temp + i_error_max_temp * Ki_temp + 5 * Kd_temp)*100;

// Variables
double err_temp, cur_temp, int_error_temp,current_time, elapsed_time, previous_time=0, deriv_error_temp, previous_error_temp, RA_deriv_error_temp, PID_output_temp, heaterPWM;
double P_temp, I_temp, D_temp;

void setup(){
  // Serial Monitor
  Serial.begin(115200);
  while (!Serial);

  // Setup infrared sensor
  mlx.begin();

  // Setup heating pad and fans
  pinMode(HEATERPIN,OUTPUT);
  pinMode(FANPIN,OUTPUT);
}

void loop(){
  Serial.print("Temperature = ");
  cur_temp = mlx.readObjectTempC();
  Serial.println(cur_temp);
  delay(500);

  // PID Loop
  current_time = millis()*1000;
  elapsed_time = current_time - previous_time;
  previous_time = current_time;

  // Input set point
  if (Serial.available()) {
    String setPoint = Serial.readStringUntil('\n');
    setPoint_temp = setPoint.toFloat();
  }
  // error calculation
  err_temp = setPoint_temp - cur_temp;

  // PID errors caculation
  int_error_temp = int_error_temp + err_temp * elapsed_time;  
  deriv_error_temp = (err_temp - previous_error_temp) / elapsed_time;  
  previous_error_temp = err_temp;

  // Derivative Term Max/Min Checking
  if (abs(deriv_error_temp) > d_error_max_temp)
  {
    if (deriv_error_temp > 0)
      deriv_error_temp = d_error_max_temp;
    else
      deriv_error_temp = -d_error_max_temp;
  }

  // Output Assembly
  P_temp = Kp_temp * err_temp;
  I_temp = Ki_temp * int_error_temp;
  D_temp = Kd_temp * deriv_error_temp;
  PID_output_temp = P_temp + I_temp + D_temp;

  // Output Min/Max Checking
  if (abs(PID_output_temp) > PID_Output_max_temp)
  {
    if (PID_output_temp > 0)
      PID_output_temp = PID_Output_max_temp;
    else
      PID_output_temp = -PID_Output_max_temp;
  }
  
  // Control heating pad and fans
    heaterPWM = map(PID_output_temp,-PID_Output_max_temp,PID_Output_max_temp,1,255);
    analogWrite(HEATERPIN,heaterPWM);
    analogWrite(FANPIN,255);
}
