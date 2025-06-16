//include libraries
#include<FEHLCD.h>
#include<FEHIO.h>
#include<FEHMotor.h>
#include<FEHServo.h>
#include<FEHUtility.h>
#include<math.h>
#include<FEHSD.h>
#include<FEHBuzzer.h>
#include<FEHFile.h>
#include<FEHRCS.h>
#include <FEHRandom.h>
#include<FEHBattery.h>

//initialize ports
FEHMotor motorBack(FEHMotor::Motor0,9.0);
FEHMotor motorRight(FEHMotor::Motor1,9.0);
FEHMotor motorLeft(FEHMotor::Motor2,9.0);
FEHServo arm(FEHServo::Servo1);
AnalogInputPin lightReader(FEHIO::P0_2);
DigitalEncoder encoder(FEHIO::P0_0);
AnalogInputPin right(FEHIO::P2_6);
AnalogInputPin middle(FEHIO::P2_3);
AnalogInputPin left(FEHIO::P2_1);

//switches
DigitalInputPin switch1(FEHIO::P0_5); // front
DigitalInputPin switch2(FEHIO::P0_7); // left
DigitalInputPin switch3(FEHIO::P1_0); // right
DigitalInputPin switch4(FEHIO::P1_7); // back

//declare function headers
void driveForward(float);
void driveBackward(float);
void strafeLeft(float);
void strafeRight(float);
void spinRight(float);
void spinLeft(float);
void driveForwardLeft(float);

//movement states
enum Movement
{
    FORWARD,
    BACKWARD,
    STRAFE_LEFT,
    STRAFE_RIGHT,
    SPIN_LEFT,
    SPIN_RIGHT,
    FORWARD_LEFT
};

void untilSwitch(float,Movement,DigitalInputPin);
void untilTime(float,Movement,float);
void untilCount(float,Movement,int);
void cycleActions();
void ticketKiosk(float,int);
void fuelLever(float,int);
void fuelLeverToLight(float);
void fuelLeverPt2(float);
void fourthPoint(float);
void luggageDrop(float);
void upCarefully(int, int, float);
void downCarefully(int, int, float);
void carefulBackAndForth();
void backAndForth();

// Arm
// MIN: 500
// MAX: 2152

// TEAM_KEY: D5TsTk92Q

int main()
{   
    
    LCD.WriteLine(Battery.Voltage());
    Sleep(2.0);
    arm.SetMin(500);
    arm.SetMax(2152);
  
    //backAndForth();
    arm.SetDegree(90);

    RCS.InitializeTouchMenu("D5TsTk92Q");
    
/*
    int correctLever = Random.RandInt()%3;
    if(correctLever >2||correctLever<0){
        correctLever = 1;
    }*/
    
    int correctLever = RCS.GetCorrectLever();
    //int correctLever = 1;
    // TODO: Add time start
    
    //wait for light
    while (lightReader.Value() >= 1.0)
    {
        LCD.Clear();
        LCD.WriteAt(lightReader.Value(), 100, 100);
    }

    //full course
    luggageDrop(75);
    LCD.WriteAt(correctLever, 100, 100);
 
    fuelLever(75,correctLever);
    fuelLeverToLight(75);
    fourthPoint(75);
}

//luggage drop function
void luggageDrop(float speed)
{
    untilSwitch(45,STRAFE_LEFT,switch2); // backwards
    untilCount(75,STRAFE_RIGHT,29);
    untilCount(50,SPIN_RIGHT,4);
    untilSwitch(90,STRAFE_RIGHT,switch3);
    //untilTime(40,STRAFE_RIGHT,.25);
    //untilCount(85,STRAFE_LEFT,1);
    Sleep(.5);
    arm.SetDegree(115);
    Sleep(.75);
    arm.SetDegree(90);
    untilCount(85,STRAFE_LEFT,8);//Strafe 
}

//code for fuel lever task
void fuelLever(float speed,int correctLever)
{
    //move from luggage drop to fuel lever
    //untilCount(40,SPIN_RIGHT,12);
    untilSwitch(50, FORWARD, switch1);
    untilTime(50, BACKWARD, .1);
    untilCount(40,SPIN_RIGHT,11);

    //untilSwitch(50,STRAFE_LEFT,switch2);
    //untilTime(20,STRAFE_LEFT,.10);

    // Check which lever to flip and perform some action
    if (correctLever==0)
    {
        untilCount(30,STRAFE_RIGHT,20);
    }
    else if (correctLever==1)
    {
        untilCount(30,STRAFE_RIGHT,15);
    }
    else if (correctLever==2)
    {
        untilCount(30,STRAFE_RIGHT,6);
    }
    untilCount(20,SPIN_LEFT,11);
    //flick lever down
    arm.SetDegree(5);
    
    //arm.SetDegree(5);
    Sleep(.75);
    //move back and wait
   
    arm.SetDegree(60);
    untilCount(30, SPIN_LEFT, 11);
    arm.SetDegree(0);

    untilCount(30, STRAFE_RIGHT, 9);
    if(correctLever==2){
        untilCount(30, STRAFE_LEFT, 1);
    }
    Sleep(2.0);
    //move forward and flip up
    untilTime(30, FORWARD, 1.6);
    Sleep(.25);
    untilTime(30, BACKWARD, 1.2);
    //move back and set arm back up
    arm.SetDegree(60);
    //arm.SetDegree(90);
    Sleep(1.0);
}

//code for movement from fuel lever to reading in light for kiosk
void fuelLeverToLight(float speed)
{
    //move up ramp
    untilSwitch(40,STRAFE_RIGHT,switch3);
    untilCount(50,STRAFE_LEFT,4);
    untilTime(95,BACKWARD,1.5);
    untilCount(50,SPIN_LEFT,19); //reorient to front
    untilSwitch(60,STRAFE_LEFT,switch2);
    untilTime(30,STRAFE_LEFT,.5);
    untilCount(50,STRAFE_RIGHT,14);
    untilTime(50,FORWARD,.5);
    LCD.Clear();
    LCD.WriteAt("Forward",150,100);
    //read in light
    while (lightReader.Value()>2)
    {
        driveForward(25);
    }
    motorBack.Stop();
    motorRight.Stop();
    motorLeft.Stop();
    Sleep(1.0);
    int color=-1;
    //set color from light
    if (lightReader.Value()<=1.2)
    {
        LCD.SetBackgroundColor(RED);
        LCD.WriteAt(lightReader.Value(),150,50);
        LCD.WriteAt("RED",150,100);
        color=1;
    }
    else if (lightReader.Value()>1.2 && lightReader.Value()<2.5)
    {
        LCD.SetBackgroundColor(BLUE);
        LCD.WriteAt(lightReader.Value(),150,50);
        LCD.WriteAt("BLUE",150,100);
        color=2;
    }
    else
    {
        LCD.SetBackgroundColor(BLACK);
        LCD.WriteAt(lightReader.Value(),150,50);
        LCD.WriteAt("NO COLOR READ",150,100);
    }
    untilTime(50,BACKWARD,.5);
    ticketKiosk(75,color);
}

//code for ticket kiosk
void ticketKiosk(float speed,int color)
{
    //GO TO CORRECT BUTTON
    if (color==1) // RED
    {
        untilCount(50,STRAFE_RIGHT,22);
    }
    else // BLUE
    {
        untilCount(50,STRAFE_RIGHT,14);
    }
    untilSwitch(50,FORWARD,switch1);

    untilTime(50,BACKWARD,.6);

    //GO TO WHITE BUTTON
    if (color==1) // RED
    {
        untilCount(50, STRAFE_LEFT, 8);
        untilCount(50,SPIN_RIGHT,10);
        Sleep(.15);
        untilCount(30,STRAFE_LEFT,14);
        //untilSwitch(30,STRAFE_LEFT,switch2);
        //downCarefully(90, 40, .01);
        arm.SetDegree(33);
        Sleep(.35);
        arm.SetDegree(90);
        
   
        untilCount(speed,STRAFE_RIGHT,2);
        arm.SetDegree(179);
        untilTime(40,FORWARD,.80);
    }
    else // BLUE
    {
        untilCount(50,SPIN_RIGHT,10);
        untilTime(50, BACKWARD, .275);
        Sleep(.15);
        untilCount(30,STRAFE_LEFT,14);
        //untilSwitch(30,STRAFE_LEFT,switch2);
        //downCarefully(90, 40, .01);
        
        arm.SetDegree(33);
     
        Sleep(.35);
        arm.SetDegree(90);

        untilCount(speed,STRAFE_RIGHT,2);
        arm.SetDegree(179);
        Sleep(.1);
        untilTime(40,FORWARD,1.0);
        untilTime(20, BACKWARD, .15);
    }
}
//code for passport section
void fourthPoint(float speed)
{

    //move forward
    arm.SetDegree(68);
   // downCarefully(180, 68, .05);
    //arm.SetDegree(70);
    Sleep(1.5);
    untilTime(speed,BACKWARD,.25);
    untilCount(speed, STRAFE_LEFT, 2);
    //flip passport 
    //downCarefully(68, 40, .05);
    arm.SetDegree(40);
    Sleep(.5);
    untilTime(speed,FORWARD,.45);
    //flip back
    //upCarefully(40, 130, .05);
    arm.SetDegree(130);
    Sleep(.5);
    //go hit red button
    untilTime(speed,BACKWARD,.5);
    arm.SetDegree(100);
    untilCount(speed,STRAFE_RIGHT,30);
    untilCount(speed,SPIN_LEFT,10);
    untilSwitch(speed,STRAFE_RIGHT,switch3);
    untilTime(speed,STRAFE_RIGHT,.1);
    Sleep(.5);
    untilCount(speed,STRAFE_LEFT,4);
    untilTime(speed,BACKWARD,4);
}

//does action passed in until count is reached
void untilCount(float speed,Movement action,int counts)
{
    // Reset encoder counts
    LCD.Clear();
    encoder.ResetCounts();
    while (encoder.Counts()<counts)
    {
        switch (action)
        {
        case FORWARD:
            driveForward(speed);
            LCD.WriteAt("Forward",150,100);
            break;
        case BACKWARD:
            driveBackward(speed);
            LCD.WriteAt("Backward",150,100);
            break;
        case STRAFE_LEFT:
            strafeLeft(speed);
            LCD.WriteAt("Strafe Left",150,100);
            break;
        case STRAFE_RIGHT:
            strafeRight(speed);
            LCD.WriteAt("Strafe Right",150,100);
            break;
        case SPIN_LEFT:
            spinLeft(speed);
            LCD.WriteAt("Spin Left",150,100);
            break;
        case SPIN_RIGHT:
            spinRight(speed);
            LCD.WriteAt("Spin Right",150,100);
            break;
        case FORWARD_LEFT:
            driveForwardLeft(speed);
            LCD.WriteAt("Forward Left",150,100);
            break;
        }
    }
    motorRight.Stop();
    motorLeft.Stop();
    motorBack.Stop();
}

//does action passed in until a switch is pressed
void untilSwitch(float speed,Movement action,DigitalInputPin button)
{
    LCD.Clear();
    while (button.Value()>0)
    {
        switch (action)
        {
        case FORWARD:
            driveForward(speed);
            LCD.WriteAt("Forward",150,100);
            break;
        case BACKWARD:
            driveBackward(speed);
            LCD.WriteAt("Backward",150,100);
            break;
        case STRAFE_LEFT:
            strafeLeft(speed);
            LCD.WriteAt("Strafe Left",150,100);
            break;
        case STRAFE_RIGHT:
            strafeRight(speed);
            LCD.WriteAt("Strafe Right",150,100);
            break;
        case SPIN_LEFT:
            spinLeft(speed);
            LCD.WriteAt("Spin Left",150,100);
            break;
        case SPIN_RIGHT:
            spinRight(speed);
            LCD.WriteAt("Spin Right",150,100);
            break;

        case FORWARD_LEFT:
            driveForwardLeft(speed);
            LCD.WriteAt("Forward Left",150,100);
            break;
        }
    }
    motorBack.Stop();
    motorLeft.Stop();
    motorRight.Stop();
}

//performs action until a certain amount of time is reached
void untilTime(float speed,Movement action,float time)
{
    LCD.Clear();
    float t_now=TimeNow();
    switch (action)
    {
    case FORWARD:
        driveForward(speed);
        LCD.WriteAt("Forward",150,100);
        break;
    case BACKWARD:
        driveBackward(speed);
        LCD.WriteAt("Backward",150,100);
        break;
    case STRAFE_LEFT:
        strafeLeft(speed);
        LCD.WriteAt("Strafe Left",150,100);
        break;
    case STRAFE_RIGHT:
        strafeRight(speed);
        LCD.WriteAt("Strafe Right",150,100);
        break;
    case SPIN_LEFT:
        spinLeft(speed);
        LCD.WriteAt("Spin Left",150,100);
        break;
    case SPIN_RIGHT:
        spinRight(speed);
        LCD.WriteAt("Spin Right",150,100);
        break;
    }
    while (TimeNow()-t_now<time)
    {
    }
    motorBack.Stop();
    motorLeft.Stop();
    motorRight.Stop();
}

//BELOW ARE SIMPLE MOVEMENT PROGRAMS
void driveForwardLeft(float speed)
{
    motorBack.SetPercent(speed);
    motorRight.SetPercent(-speed);
}

void driveForward(float speed)
{
    motorRight.SetPercent(-speed);
    motorLeft.SetPercent(speed);
}

void driveBackward(float speed)
{
    motorRight.SetPercent(speed);
    motorLeft.SetPercent(-speed);
}

void strafeLeft(float speed)
{
    motorBack.SetPercent(speed);
    // both clockwise
    motorRight.SetPercent(-.5*(speed));
    motorLeft.SetPercent(-.5*(speed));
}

void strafeRight(float speed)
{
    motorBack.SetPercent(-speed);
    // both counter-clockwise
    motorRight.SetPercent(.5*speed);
    motorLeft.SetPercent(.5*speed);
}

void spinRight(float speed)
{
    motorBack.SetPercent(speed);
    motorRight.SetPercent(speed);
    motorLeft.SetPercent(speed);
}

void spinLeft(float speed)
{
    motorBack.SetPercent(-speed);
    motorRight.SetPercent(-speed);
    motorLeft.SetPercent(-speed);
}

//cycle between movements for testing purposes
void cycleActions()
{
    float x;
    float y;
    int count=-1;
    while (true)
    {
        if (LCD.Touch(&x,&y))
        {
            count++;
            LCD.Clear();
            while (LCD.Touch(&x,&y))
            {
                switch (count%6)
                {
                case 0:
                    driveForward(50);
                    LCD.WriteAt("Forward",150,100);
                    break;
                case 1:
                    driveBackward(50);
                    LCD.WriteAt("Backward",150,100);
                    break;
                case 2:
                    strafeLeft(50);
                    LCD.WriteAt("Strafe Left",150,100);
                    break;
                case 3:
                    strafeRight(50);
                    LCD.WriteAt("Strafe Right",150,100);
                    break;
                case 4:
                    spinLeft(50);
                    LCD.WriteAt("Spin Left",150,100);
                    break;
                case 5:
                    spinRight(50);
                    LCD.WriteAt("Spin Right",150,100);
                    break;
                }
            }
        }
        motorBack.Stop();
        motorRight.Stop();
        motorLeft.Stop();
        Sleep(.1);
    }
}

//move servo back and forth for testing
void backAndForth()
{
    while (true)
    {
        arm.SetDegree(0);
        Sleep(2.0);
        arm.SetDegree(180);
        Sleep(2.0);
    }
}

//move servo slowly
void upCarefully(int current, int degree, float sleep){
    while(current<degree && current != 180){
        current++;
        arm.SetDegree(current);
        Sleep(sleep);
    }
}

//move servo slowly
void downCarefully(int current, int degree, float sleep){
    while(current>degree && current != 0){
        current--;
        arm.SetDegree(current);
        Sleep(sleep);
    }
}

//move servo back and forth for testing
void carefulBackAndForth()
{
    arm.SetDegree(0);
    while (true)
    {
        upCarefully(0, 180, .05);
        Sleep(1.0);
        downCarefully(180, 0, .05);
        Sleep(1.0);
    }
}