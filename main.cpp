#include "mbed.h"

AnalogIn my_analog_pin1{PA_4};
AnalogIn my_analog_pin2{PA_5};
DigitalOut led{LED1};

Timer my_timer;
bool is_implausible = false;     //If implausiblity has occured
bool is_running = true;          //If there should be power going to the motor
int main()
{
   while(true) 
   {
      double sensor1V = my_analog_pin1.read() *3.3;
      double sensor2V = my_analog_pin2.read() *3.3;
      //Convert the voltages to pedal position percentages
      //These equations are based off the provided table
      double pos1 = 0.5 * sensor1V - 0.125;
      double pos2 = (5.0 / 12.0) * sensor2V - 0.125;

      ThisThread::sleep_for(1s);
      if(pos1 >= 0.1|| pos2 >= 0.1) //Checks if pedals are past (position = 0.1)
      {
         if((fabs(pos1-pos2)/2.0)*100 > 10) //Check if positions differ by more than 10% 
         {
            if(!is_implausible) //First instance being implausbile
            {
               //starts the timer
               my_timer.start();
               printf("Implausability Started\n");
               is_implausible = true;
            }
            int64_t timer_ms= my_timer.elapsed_time().count()/1000; //microseconds to miliseconds
            if(timer_ms> 100) //if implausibility has occured for longer than 100msec
            {
               printf("Implausibility occured for longer than 100msec: %lld ms\n", timer_ms);
               printf("0");
               is_running = false;
               ThisThread::sleep_for(10s); //shutdown power to motor
            }
         }
         else
         {
            //not implausible
            is_implausible = false;
            my_timer.stop();
            my_timer.reset();
         }
      }
      else
      {
         //if pos1 and pos2 do not differ by a minimum of 10%
         //Also not implausbible
         is_implausible = false;
         my_timer.stop();
         my_timer.reset();
      }

      //Check for open/short circuit
      
      //Average them out
      float average = (pos1+pos2)/2;
      float voltAverage = fabs(sensor1V-sensor2V)/2.0*100;
      printf("Average of the sensor positions are %.5f, \n", average);
      printf("Difference of the voltages are %.5f %, \n", voltAverage);
      // printf("Sensor 1 Voltage is: %lf, \n", sensor1V);
      // printf("Sensor 2 Voltage is: %lf, \n", sensor2V);
      if(is_running)
      {
         //
      }
      // all in one line
      // double average = (1/2 * my_analog_pin1.read() - 1/8) + (5/12 * my_analog_pin2.read() - 1/8)/2

   }
   //computer the desired torque demand value
   // main() is expected to loop forever.
   // If main() actually returns the processor will halt
   return 0;
}

