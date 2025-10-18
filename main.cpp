#include "mbed.h"

AnalogIn my_analog_pin1{PA_4}; //APPS0
AnalogIn my_analog_pin2{PA_5}; //APPS1
AnalogIn my_analog_pin3{PA_6}; //Brake Sensor
DigitalIn my_digital_input{PA_7}; //Cockpit switch
DigitalOut my_digital_output{PA_0}; //Buzzer pin
DigitalOut led{LED1};

Timer my_timer;
Timer RTD_timer;
bool is_implausible = false;     //If implausiblity has occured
bool isRunning = false;          //If power is going to the motor
bool buzzerSounding = false;     //If the buzzer is sounding
bool RTD = false;                //If in Ready To Drive sequence

int main()
{
   while(true) 
   {
      double sensor1V = my_analog_pin1.read() *3.3;
      double sensor2V = my_analog_pin2.read() *3.3;
      double sensor3V = my_analog_pin3.read() *3.3;     
      //Convert the voltages to pedal position percentages
      //These equations are based off the provided table
      double pos1 = 0.5 * sensor1V - 0.125;
      double pos2 = (5.0 / 12.0) * sensor2V - 0.125;
      //Not sure what brake percentages are so I'm just using the same as APP0
      double breakPos = 0.5 * sensor3V - 0.125;

      //check if voltages don't go out of their expected ranges
      bool expectedRange0 = 0.25 < sensor1V && sensor1V < 2.25;
      bool expectedRange1 = 0.3 < sensor1V && sensor2V < 2.7;
   


      if(isRunning && (pos1 >= 0.1|| pos2 >= 0.1 || !expectedRange0 || !expectedRange1)) //Checks if pedals are past (position = 0.1) or APPs are not in their expected voltage range
      {
         if((fabs(pos1-pos2)/2.0)*100 > 10 || !expectedRange0 || !expectedRange1 ) //Check if positions differ by more than 10%  or APPs are not in their expected voltage range
         {
            if(!is_implausible) //First instance being implausbile
            {
               //starts the timer
               my_timer.start();
               printf("Implausibility Started\n");
               is_implausible = true;
            }
            int64_t timer_ms = my_timer.elapsed_time().count()/1000; //microseconds to miliseconds
            if(timer_ms > 100) //if implausibility has occured for longer than 100msec
            {
               printf("Implausibility occured for longer than 100msec: %lld ms\n", timer_ms);
               printf("0");
               isRunning = false;
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
         //not implausbile
         //Motor is not running or (pos below 10%, and voltage range within expected ranges)
         is_implausible = false;
         my_timer.stop();
         my_timer.reset();
      }
      if(isRunning)
      {
         //Average them out
         float average = (pos1+pos2)/2;
         float voltAverage = fabs(sensor1V-sensor2V)/2.0*100;
         // printf("Average of the sensor positions are %.5f, \n", average);
         // printf("Difference of the voltages are %.5f %, \n", voltAverage);
         // printf("Sensor 1 Voltage is: %lf, \n", sensor1V);
         // printf("Sensor 2 Voltage is: %lf, \n", sensor2V);
         //Print the throttle output (not really sure what this is supposed to be) I assume it correlates to the pedal position average in some way
         printf("Throttle output is:  %.5f, \n", average);
      }
      //Driver must press brakes - use another identical APPS sensor for this (wait until it’s ~80%)
      if(breakPos >= 0.8)
      {
         //Driver must press cockpit switch - use a DigitalIn for this (wait until it becomes high)

         if(my_digital_input.read() == 1)
         {
            //Sound RTD buzzer for 1s - use a DigitalOut for this (set it to high for 1s)
            RTD = true;

         }
      }
      if(RTD == true && !isRunning && !buzzerSounding)
      {
         RTD_timer.start();
         my_digital_output.write(1);
         buzzerSounding = true;
         RTD = false;
      }
      if(buzzerSounding)
      {
         int64_t timer_ms = RTD_timer.elapsed_time().count()/1000; //microseconds to miliseconds
         if(timer_ms > 1000)
         {
            //more than 1 sec have passed
            my_digital_output.write(0);
            buzzerSounding = false;
            isRunning = true;
         }
      }
      if(!RTD || !isRunning)
      {
         printf("0");
      }
   }
   // compute the desired torque demand value
   // main() is expected to loop forever.
   // If main() actually returns the processor will halt
   return 0;
}

