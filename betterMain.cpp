#include "mbed.h"

typedef enum
{
    INITIAL,
    RTDSEQUENCE,
    RUNNING,
    IMPLAUSIBLE,
    STOP
} ETCState;

typedef struct
{
    ETCState state;
    Timer implausible_timer;
    Timer RTD_timer;
    
} ETCData;

AnalogIn my_analog_pin1{PA_4}; //APPS0
AnalogIn my_analog_pin2{PA_5}; //APPS1
AnalogIn my_analog_pin3{PA_6}; //Brake Sensor
DigitalIn my_digital_input{PA_7}; //Cockpit switch
DigitalOut my_digital_output{PA_0}; //Buzzer pin

ETCData ETC;



int main()
{
    while(true){
        switch(ETC.state)
        {
            case INITIAL:
            {
                //Not sure what brake percentages are so I'm just using the same as APP0
                double breakPos = 0.5 * sensor3V - 0.125;
                if(breakPos > 0.8) //Brake position is past 80%
                {
                    if(my_digital_input == 1) //Cockpit switch is 1
                    {
                        //Start the Ready to drive sequence and sound the buzzer
                        ETC.state = RTDSEQUENCE;
                        ETC.RTD_timer.start();
                        my_digital_output.write(1); //sound the buzzer
                    }
                }
                printf("0");
            }
            case RTDSEQUENCE:
            {
                //Sound buzzer for one second
                int64_t timer_ms = ETC.RTD_timer.elapsed_time().count()/1000; //microseconds to miliseconds
                if(timer_ms > 1000)
                {
                    //more than 1 sec have passed
                    my_digital_output.write(0); //unsound the buzzer
                    ETC.state = RUNNING;
                }
            }
            case RUNNING:
            {
                double sensor1V = my_analog_pin1.read() *3.3;
                double sensor2V = my_analog_pin2.read() *3.3;
                double pos1 = 0.5 * sensor1V - 0.125;
                double pos2 = (5.0 / 12.0) * sensor2V - 0.125;
                bool expectedRange0 = 0.25 < sensor1V && sensor1V < 2.25;
                bool expectedRange1 = 0.3 < sensor1V && sensor2V < 2.7;
                //check for implausibility
                if(!expectedRange0 || !expectedRange1)
                {
                    //not in expected ranges
                    //start implausibility
                    ETC.implausible_timer.start();
                    ETC.state = IMPLAUSIBLE;

                }
                else if(pos1 >= 0.1|| pos2 >= 0.1 ||)
                {
                    if((fabs(pos1-pos2)/2.0)*100 > 10)
                    {
                        //Pedal positions differ more than 10%
                        //start implausibility
                        ETC.implausible_timer.start();
                        ETC.state = IMPLAUSIBLE;
                    }
                }
            }
            case IMPLAUSIBLE:
            {
                double sensor1V = my_analog_pin1.read() *3.3;
                double sensor2V = my_analog_pin2.read() *3.3;
                double pos1 = 0.5 * sensor1V - 0.125;
                double pos2 = (5.0 / 12.0) * sensor2V - 0.125;
                bool expectedRange0 = 0.25 < sensor1V && sensor1V < 2.25;
                bool expectedRange1 = 0.3 < sensor1V && sensor2V < 2.7;

                //check if positions are plausible
                if(expectedRange0 && expectedRange1)
                {
                    //if positions are less than 10%
                    if(pos1 <= 0.1  && pos2 <= 0.1)
                    {
                        ETC.state = RUNNING;
                        ETC.implausible_timer.stop();
                        ETC.implausible_timer.reset();
                    }
                    //positions are within 10%
                    else if((fabs(pos1-pos2)/2.0)*100 <= 10)
                    {
                        ETC.state = RUNNING;
                        ETC.implausible_timer.stop();
                        ETC.implausible_timer.reset();
                    }
                }
                //If it reaches here, it is still implausible
                int64_t timer_ms = my_timer.elapsed_time().count()/1000; //microseconds to miliseconds
                if(timer_ms > 100) //if implausibility has occured for longer than 100msec
                {
                    printf("Implausibility occured for longer than 100msec: %lld ms\n", timer_ms);
                    printf("0");
                    ETC.state = STOP;
                }
            }
            case STOP:
            {
                printf("Shut down power to motor");
            }
        }
    }
}