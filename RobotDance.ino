// TODO: predpokladame ze ked sa strci kabel, zacina custom dance, a pri vytiahnuti kabla sa zapise do eepromky
#include <Servo.h>
class motor : public Servo
{
  public:
   void go(int percentage){
    if(percentage > 100) percentage = 100;
    if(percentage < -100) percentage = -100; 
    writeMicroseconds(1500+percentage*2);
   }
  
};

enum robot_state
{
  waiting_for_start_state,
  doing_choreography_state,
  choreography_done_state,
  returning_to_start_state
};

enum turning_direction{
  left,
  right
};

enum parsing_input{
  reading_starting_coordinate_state,
  reading_coordinate_state,
  reading_time_state,
  malformed_input_state
};

typedef struct coordinate{
    char first; //x-coordinate
    char second; //y-coordinate
    unsigned int wait; //when to move next
};

int leftSensor,middleSensor,rightSensor, edgeSensors, mostLeftSensor, mostRightSensor;
motor leftMotor,rightMotor;
int MotorPower = 30;
parsing_input parsing_input_state;
char startingOrientation;
coordinate start_position;
coordinate current_position;
char current_orientation;
coordinate dance_choreography [100];
unsigned long starting_time;


#include <EEPROM.h>
#include <Arduino.h>  // for type definitions

template <class T> int EEPROM_write(int ee, const T& value)
{
    const byte* p = (const byte*)(const void*)&value;
    unsigned int i;
    for (i = 0; i < sizeof(value); i++)
          EEPROM.write(ee++, *p++);
    return i;
}

template <class T> int EEPROM_read(int ee, T& value)
{
    byte* p = (byte*)(void*)&value;
    unsigned int i;
    for (i = 0; i < sizeof(value); i++)
          *p++ = EEPROM.read(ee++);
    return i;
}

void readSensors()
{
    leftSensor = digitalRead(4);
    middleSensor = digitalRead(5);
    rightSensor = digitalRead(6);
    mostLeftSensor = digitalRead(3);
    mostRightSensor = digitalRead(7);
}

void turn_in_place(turning_direction direction){
    bool turningDone = false;
    bool isOnWhite = false;
    bool ignoreMarksWhenTurningStartedMS = 500;
    unsigned long turningStart = millis();
    while(!turningDone)
    {
        readSensors();
        if(direction == left){
            leftMotor.go(-MotorPower);
            rightMotor.go(-MotorPower);
            if(middleSensor == 1 && leftSensor == 1 && rightSensor == 1 && millis()-turningStart > ignoreMarksWhenTurningStartedMS){
                isOnWhite = true;
            }
            if(isOnWhite && middleSensor == 0){
                turningDone = true;
                // turn just a little bit more to be straight
                unsigned long timer_start = millis();
                while(millis() - timer_start < 32){
                  leftMotor.go(-MotorPower);
                  rightMotor.go(-MotorPower);
                }
            }
        }
        if(direction == right){
            leftMotor.go(MotorPower);
            rightMotor.go(MotorPower);
            if(middleSensor == 1 && leftSensor == 1 && rightSensor == 1 && millis()-turningStart > ignoreMarksWhenTurningStartedMS){
                isOnWhite = true;
            }
            if(isOnWhite && middleSensor == 0){
                turningDone = true;
                // turn just a little bit more to be straight
                unsigned long timer_start = millis();
                while(millis() - timer_start < 32){
                  leftMotor.go(MotorPower);
                  rightMotor.go(MotorPower);
                }
            }
        }
    }
    leftMotor.go(0);
    rightMotor.go(0);
}

void go(int x) {
    bool isOnWhite = false;
    int numberOfPassedFields = 0;
    while(numberOfPassedFields < 1){   
        readSensors(); 
        if(middleSensor == 0){
            // go straight
            leftMotor.go(MotorPower);
            rightMotor.go(-MotorPower);
        }   
        else{
            if(leftSensor == 0){
                //turn left
                leftMotor.go(MotorPower/3);
                rightMotor.go(-MotorPower);
            }
            else{
                if(rightSensor == 0){
                    //turn right
                    leftMotor.go(MotorPower);
                    rightMotor.go(-MotorPower/3);
                }
                else{
                    // black line is between sensors
                    leftMotor.go(MotorPower);
                    rightMotor.go(-MotorPower);
                }
            }
        }
        if(mostLeftSensor == 1 && mostRightSensor == 1){
            isOnWhite = true;
        }
        if(isOnWhite = true && (mostLeftSensor == 0 || mostRightSensor == 0)){
            isOnWhite = false;
            numberOfPassedFields++;
        }
    }
    // the rotation axis has to be in the crossroads
    unsigned long timer_start = millis();
    while(millis() - timer_start < 340){
        leftMotor.go(MotorPower);
        rightMotor.go(-MotorPower);
    }
    leftMotor.go(0);
    rightMotor.go(0);
}

void go_to_coordinate(coordinate target_coordinate){
    if(target_coordinate.first >= 'A' && target_coordinate.first <= 'Z'){
        horizontal_move(target_coordinate.first);
        vertical_move(target_coordinate.second);
    }
    else{
        vertical_move(target_coordinate.first);
        horizontal_move(target_coordinate.second);
    }
    if(millis()-starting_time < target_coordinate.wait * 100){
        delay(target_coordinate.wait * 100 - millis()-starting_time);
    }
}

void turn_light_on(unsigned int timeInMs){
    unsigned int start = millis();
    while(millis()-start < timeInMs){
        digitalWrite(11,1);
    }
    digitalWrite(11,0);
}

void go_to_start_position(){
    go_to_coordinate(start_position);
    switch(current_orientation){
        case 'N':
            switch(startingOrientation){
                case 'N':
                    break;
                case 'E':
                    turn_in_place(right);
                    break;
                case 'W':
                    turn_in_place(left);
                    break;
                case 'S':
                    if(get_current_horizontal_position() == 'A'){
                        turn_in_place(right);
                        turn_in_place(right);
                    }
                    else{
                        turn_in_place(left);
                        turn_in_place(left);
                    }
                    break;
            }
            break;
        case 'S':
            switch(startingOrientation){
                case 'S':
                    break;
                case 'W':
                    turn_in_place(right);
                    break;
                case 'E':
                    turn_in_place(left);
                    break;
                case 'N':
                    if(get_current_horizontal_position() == 'A'){
                        turn_in_place(left);
                        turn_in_place(left);
                    }
                    else{
                        turn_in_place(right);
                        turn_in_place(right);
                    }
                    break;
            }
            break;
        case 'W':
            switch(startingOrientation){
                case 'W':
                    break;
                case 'S':
                    turn_in_place(right);
                    break;
                case 'N':
                    turn_in_place(left);
                    break;
                case 'E':
                    if(get_current_vertical_position() == '1'){
                        turn_in_place(right);
                        turn_in_place(right);
                    }
                    else{
                        turn_in_place(left);
                        turn_in_place(left);
                    }
                    break;
            }
            break;
        case 'E':
            switch(startingOrientation){
                case 'E':
                    break;
                case 'S':
                    turn_in_place(left);
                    break;
                case 'N':
                    turn_in_place(right);
                    break;
                case 'W':
                    if(get_current_vertical_position() == '1'){
                        turn_in_place(left);
                        turn_in_place(left);
                    }
                    else{
                        turn_in_place(right);
                        turn_in_place(right);
                    }
                    break;
            }
            break;
    }
    current_orientation = startingOrientation;
}

void go_steps(int numberOfFieldsToGo){
    for(int i=0; i < numberOfFieldsToGo; i++){
        go(1);
    }
}

void horizontal_move(char target_position){
    char current_horizontal_position = get_current_horizontal_position();
    char current_vertical_position = get_current_vertical_position();
    if(current_horizontal_position == target_position){
        return;
    }
    if(current_horizontal_position < target_position){
        //go right 
        int number_of_steps = target_position - current_horizontal_position;
        if(current_orientation == 'E'){
            go_steps(number_of_steps);
        }
        if(current_orientation == 'N'){
            turn_in_place(right);
            current_orientation = 'E';
            go_steps(number_of_steps);
        }
        if(current_orientation == 'S'){
            turn_in_place(left);
            current_orientation = 'E';
            go_steps(number_of_steps);
        }
        if(current_orientation == 'W'){
            if(current_vertical_position == '1'){
                turn_in_place(right);
                turn_in_place(right);
                current_orientation = 'E';
                go_steps(number_of_steps);
            }
            else{
                turn_in_place(left);
                turn_in_place(left);
                current_orientation = 'E';
                go_steps(number_of_steps);
            }            
        }
    }
    if(current_horizontal_position > target_position){
        //go left
        int number_of_steps = current_horizontal_position - target_position;
        if(current_orientation == 'W'){
            go_steps(number_of_steps);
        }
        if(current_orientation == 'S'){
            turn_in_place(right);
            current_orientation = 'W';
            go_steps(number_of_steps);
        }
        if(current_orientation == 'N'){
            turn_in_place(left);
            current_orientation = 'W';
            go_steps(number_of_steps);
        }
        if(current_orientation == 'E'){
            if(current_vertical_position == '1'){
                turn_in_place(left);
                turn_in_place(left);
                current_orientation = 'W';
                go_steps(number_of_steps);
            }
            else{
                turn_in_place(right);
                turn_in_place(right);
                current_orientation = 'W';
                go_steps(number_of_steps);
            }            
        }
    }
    set_current_horizontal_position(target_position);
}

void set_current_horizontal_position(char target_position){
    if(current_position.first >= 'A' && current_position.first <= 'Z'){
        current_position.first = target_position;
    }
    else{
        current_position.second = target_position;
    }
}

char get_current_horizontal_position(){
    if(current_position.first >= 'A' && current_position.first <= 'Z'){
        return current_position.first;
    }
    else{
        return current_position.second;
    }
}

void vertical_move(char target_position){
    char current_horizontal_position = get_current_horizontal_position();
    char current_vertical_position = get_current_vertical_position();
    if(current_vertical_position == target_position){
        return;
    }
    if(current_vertical_position < target_position){
        //go up 
        int number_of_steps = target_position - current_vertical_position;
        if(current_orientation == 'N'){
            go_steps(number_of_steps);
        }
        if(current_orientation == 'W'){
            turn_in_place(right);
            current_orientation = 'N';
            go_steps(number_of_steps);
        }
        if(current_orientation == 'E'){
            turn_in_place(left);
            current_orientation = 'N';
            go_steps(number_of_steps);
        }
        if(current_orientation == 'S'){
            if(current_horizontal_position == 'A'){
                turn_in_place(left);
                turn_in_place(left);
                current_orientation = 'N';
                go_steps(number_of_steps);
            }
            else{
                turn_in_place(right);
                turn_in_place(right);
                current_orientation = 'N';
                go_steps(number_of_steps);
            }            
        }
    }
    if(current_vertical_position > target_position){
        //go dowm
        int number_of_steps = current_vertical_position - target_position;
        if(current_orientation == 'S'){
            go_steps(number_of_steps);
        }
        if(current_orientation == 'E'){
            turn_in_place(right);
            current_orientation = 'S';
            go_steps(number_of_steps);
        }
        if(current_orientation == 'W'){
            turn_in_place(left);
            current_orientation = 'S';
            go_steps(number_of_steps);
        }
        if(current_orientation == 'N'){
            if(current_horizontal_position == 'A'){
                turn_in_place(right);
                turn_in_place(right);
                current_orientation = 'S';
                go_steps(number_of_steps);
            }
            else{
                turn_in_place(left);
                turn_in_place(left);
                current_orientation = 'S';
                go_steps(number_of_steps);
            }            
        }
    }
    set_current_vertical_position(target_position);
}

void set_current_vertical_position(char target_position){
    if(current_position.first >= '1' && current_position.first <= '9'){
        current_position.first = target_position;
    }
    else{
        current_position.second = target_position;
    }
}

char get_current_vertical_position(){
    if(current_position.first >= '1' && current_position.first <= '9'){
        return current_position.first;
    }
    else{
        return current_position.second;
    }
}

void setup() {
  Serial.begin(115200);
  //ReadDefaultChoreographyFromEEPROM();
  leftMotor.attach(12,500,2500);
  rightMotor.attach(13,500,2500);
  pinMode(3,INPUT);
  pinMode(4,INPUT);
  pinMode(5,INPUT);
  pinMode(6,INPUT);
  pinMode(7,INPUT);
  pinMode(11,OUTPUT);
}

// end of instruction mark
coordinate eoi_mark = {'q', 'q', 0};

void PrintCoordinate(coordinate coord){
  Serial.print(coord.first);
  Serial.print(' ');
  Serial.print(coord.second);
  Serial.print(' ');
  Serial.print(coord.wait);
  Serial.println(' ');
}

void ReadDefaultChoreographyFromEEPROM(){
    int reading_byte = 1;
    char startingOrientation;
    reading_byte += EEPROM_read(reading_byte, startingOrientation);
    Serial.print(reading_byte);
    Serial.println("reading starting orientation");
    Serial.println(startingOrientation);
    coordinate current_coordinate = {'a','a',0};
    while(current_coordinate.first != eoi_mark.first && current_coordinate.second != eoi_mark.second){
        Serial.println("reading coordinate...");
        reading_byte += EEPROM_read(reading_byte, current_coordinate);
        Serial.print(reading_byte);
        PrintCoordinate(current_coordinate);
    }
}

/*
 * Can contain:
 * starting position:      A1N
 * position without time:  C4 or 4C
 * time:                   T350
 */
boolean validateInputToken(char token[], parsing_input parse_state){
    String s="";
    int i;
    for(i=1;i<10;i++){
      if(token[i]=='X'){
          break;
      }
      else{
        s+=token[i];
      }
    }
//    Serial.print(s);
//    Serial.print(" ");
//    Serial.print(parse_state);
//    Serial.println();
  
   return true;
}

boolean handleSerial() {
    parsing_input parsing_state = reading_starting_coordinate_state;
    boolean ignoreWhiteSpacesAtStart=true;
    char token[10];
    byte coordinateNumber=0;
    byte counter=0;

//    Serial.println("Started parsing input!");
    
    while (Serial.available() > 0) {

        Serial.print(Serial.available());
        char incomingCharacter = Serial.read();
        Serial.print(incomingCharacter);
        Serial.print(Serial.available());
        switch(parsing_state){
            case reading_starting_coordinate_state:
                // 1. zozaciatku ignoruj whitespacy
                // 2. citaj chary
                // 3  ked pride dalsi whitespace  zvaliduj position a chod bud do malformed alebo do reading_position_state
                if (isWhitespace(incomingCharacter)&&ignoreWhiteSpacesAtStart){
                    //ignore
                }
                else if(isAlphaNumeric(incomingCharacter)){
                    ignoreWhiteSpacesAtStart=false;
                    token[counter]=incomingCharacter;
                    counter++;            
                }
                else{                                                     // was whitespace after we read first coordinate
                    boolean validStartingPosition = validateInputToken(token, parsing_state);
                    if(validStartingPosition && counter==3 ){
                        start_position.first = token[0];
                        start_position.second = token[1]; 
                        start_position.wait = 0;        
                        startingOrientation = token[2]; 

                        parsing_state=reading_coordinate_state;
                        ignoreWhiteSpacesAtStart=true;
                        counter=0;
                        memset(token, 0, sizeof(token));
                    }
                    else{
                        parsing_state=malformed_input_state;
                        return false;
                    } 
                }
                break;
            case reading_coordinate_state:
                // 1. zozaciatku ignoruj whitespacy
                // 2. citaj chary
                // 3  ked pride dalsi whitespace  zvaliduj position a a chod bud do reading_time_stat alebo do  malformed_input_state
                if (isWhitespace(incomingCharacter)&&ignoreWhiteSpacesAtStart){
                    //ignore
                }
                else if(isAlphaNumeric(incomingCharacter)){
                    ignoreWhiteSpacesAtStart=false;
                    token[counter]=incomingCharacter;
                    counter++;            
                }
                else{                                                     // was whitespace after we read first coordinate
                    boolean validCoordinate = validateInputToken(token, parsing_state);
                    if(validCoordinate && counter==2){
                        dance_choreography[coordinateNumber].first = token[0];
                        dance_choreography[coordinateNumber].second = token[1];        
                        
                        parsing_state=reading_time_state;
                        ignoreWhiteSpacesAtStart=true;
                        counter=0;
                        memset(token, 'X', sizeof(token));
                    }
                    else{
                        parsing_state=malformed_input_state;
                        return false;
                    } 
                }
                break;
            case reading_time_state:
                // 1. zozaciatku ignoruj whitespacy
                // 2. citaj chary
                // 3  ked pride dalsi whitespace zvaliduj position a a chod bud do reading_coordinate_state alebo do  malformed_input_state
                if (isWhitespace(incomingCharacter)&&ignoreWhiteSpacesAtStart){
                    //ignore
                }
                else if(isAlphaNumeric(incomingCharacter)){
                    ignoreWhiteSpacesAtStart=false;
                    token[counter]=incomingCharacter;
                    counter++;            
                }
                else{                                                     // was whitespace after we read first coordinate
                    boolean validCoordinate = validateInputToken(token, parsing_state);
                    if(validCoordinate ){
                        // token looks like T350 or T1250
                        int i;
                        String s="";
                        for(i=1;i<10;i++){
                          if(token[i]=='X'){
                              break;
                          }
                          else{
                            s+=token[i];
                          }
                        }
                        unsigned int waitTime= (unsigned int) s.toInt();
                        dance_choreography[coordinateNumber].wait = waitTime;
                                        
                        parsing_state=reading_coordinate_state;
                        ignoreWhiteSpacesAtStart=true;
                        counter=0;
                        coordinateNumber++;
                    }
                    else{
                        parsing_state=malformed_input_state;
                        return false;
                    } 
                }


                
                break;
            case malformed_input_state:
                // prestan citat zo serial liny
                break;
        }
   }

    return true;
}

// if valid (zero byte set to 1), return byte 512 as addrss of loaded choreography elde return byte 1 as address of default choreography
int select_choreography(){
    int reading_byte = 0;
    bool select_loaded_choreography;
    EEPROM_read(reading_byte, select_loaded_choreography);
    Serial.print("Zero byte: ");
    Serial.println(select_loaded_choreography);
    if(select_loaded_choreography){
        return 512;
    } 
    else{
        return 1;
    }
}

// expects already written choreography in EEPROM 
void start_dancing(){
    int reading_byte = select_choreography();
    reading_byte += EEPROM_read(reading_byte, startingOrientation);
    current_orientation = startingOrientation;
    //Serial.print(reading_byte);
    Serial.println("reading starting orientation");
    Serial.println(startingOrientation);
    coordinate current_coordinate = {'a','a',0};
    // read starting position
    reading_byte += EEPROM_read(reading_byte, current_coordinate);
    start_position = current_coordinate;
    Serial.println("start position...");
    PrintCoordinate(start_position);
    current_position= current_coordinate;
    // read dance
    while(current_coordinate.first != eoi_mark.first && current_coordinate.second != eoi_mark.second){
        Serial.println("reading coordinate...");
        reading_byte += EEPROM_read(reading_byte, current_coordinate);
        //Serial.print(reading_byte);
        PrintCoordinate(current_coordinate);
        Serial.println(" srart roling....");
        go_to_coordinate(current_coordinate);
    }
}

bool doIt = true;
// main loop
void loop() {
  int wait = 100;
  //ReadDefaultChoreographyFromEEPROM();
  start_dancing();
    //if(doIt)
    //{
        //start_position = {'1', 'A', 0};
        //startingOrientation = 'S';
        //current_orientation = 'S';
        //current_position = {'1', 'A', 0};
        //go_to_coordinate({'3','C'}); //chyba
        //return;        
        //go_steps(1);
        //turn_in_place(left);
        //turn_in_place(left);
        //current_orientation = 'S';
        //go_steps(1);
        //return;
        //turn_light_on(1000);
        //start_position = {'1', 'A', 0};
        //startingOrientation = 'W';
        //current_orientation = 'W';
        //current_position = {'1', 'A', 0};
        //go_to_coordinate({'B','3', 100});
        //turn_light_on(1000);
        //go_to_coordinate({'1','C', 200});
        //turn_light_on(1000);
        //go_to_coordinate({'3','C', 300});
        //turn_light_on(1000);
        //go_to_coordinate({'C','2', 400});
        //turn_light_on(1000);
        //go_to_coordinate({'B','3', 500});
        //turn_light_on(1000);
        //go_to_coordinate({'A','3', 600});
        //turn_light_on(1000);
        //go_to_coordinate({'1','A', 700});
        //turn_light_on(1000);
        //go_to_coordinate({'3','C', 800});
        //turn_light_on(1000);
        //go_to_start_position();        
     //}
     //doIt = false;
     return;

  
    robot_state rs = waiting_for_start_state;
    switch(rs){
      case waiting_for_start_state:
          boolean useCustomChoreography =  handleSerial();
          if(useCustomChoreography){
            // do dance with custom choreography
            // use custom
          }
          else{
            // do dance with default choreography
            // TODO: read dance from EEPROM
          }

          break;
      case doing_choreography_state:
          break;
      case choreography_done_state:
          break;
      case returning_to_start_state:
          break;
    }
}
