// function for servo
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

enum robot_state_enum{
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

typedef struct coordinate {
    char first;             // x-coordinate
    char second;            // y-coordinate
    unsigned int wait;      //when to move next
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

robot_state_enum robot_state;
parsing_input parsing_state;

unsigned int pointer_at_current_custom_choreography_byte = 512;

coordinate new_coordinate;

// end of instruction mark
coordinate eoi_mark = {'q', 'q', 0};

// functions for reading and writing to EEPROM
#include <EEPROM.h>
#include <Arduino.h>  

// input: byte where to write, value to write
// output: size of value in bytes
template <class T> int EEPROM_write(int ee, const T& value)
{
    const byte* p = (const byte*)(const void*)&value;
    unsigned int i;
    for (i = 0; i < sizeof(value); i++)
          EEPROM.write(ee++, *p++);
    return i;
}

// input: byte where to read, variablie to fill
// output: size of value in bytes
template <class T> int EEPROM_read(int ee, T& value){
    byte* p = (byte*)(void*)&value;
    unsigned int i;
    for (i = 0; i < sizeof(value); i++)
          *p++ = EEPROM.read(ee++);
    return i;
}

void setup() {
    Serial.begin(115200);
    leftMotor.attach(12,500,2500);
    rightMotor.attach(13,500,2500);

    pinMode(2,INPUT_PULLUP);  
    pinMode(3,INPUT);
    pinMode(4,INPUT);
    pinMode(5,INPUT);
    pinMode(6,INPUT);
    pinMode(7,INPUT);
    pinMode(11,OUTPUT);

    robot_state = waiting_for_start_state;
    parsing_input parsing_state = reading_starting_coordinate_state;

    // use default choreography by default
    EEPROM_write(0,false);
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
    // turn 90 degrees takes about 750ms
    int ignoreMarksWhenTurningStartedMS = 300;
    unsigned long turningStart = millis();
    while(!turningDone)
    {
        if(isOnWhite){
            // turn the light on when turning
            digitalWrite(11,1);
        }
        readSensors();
        if(direction == left){
            leftMotor.go(-MotorPower);
            rightMotor.go(-MotorPower);
            if(middleSensor == 1 && leftSensor == 1 && rightSensor == 1 && ((millis()-turningStart) > ignoreMarksWhenTurningStartedMS)){
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
            if(middleSensor == 1 && leftSensor == 1 && rightSensor == 1 && ((millis()-turningStart) > ignoreMarksWhenTurningStartedMS)){
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
    digitalWrite(11,0);
}

// go straight 1 field (there is useless parameter x because wihout it compiler doesnt want to compile for unknown reason)
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
                leftMotor.go(MotorPower/5);
                rightMotor.go(-MotorPower);
            }
            else{
                if(rightSensor == 0){
                    //turn right
                    leftMotor.go(MotorPower);
                    rightMotor.go(-MotorPower/5);
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
    // the rotation axis of robot has to be in the crossroads so go litle bit further
    unsigned long timer_start = millis();
    while(millis() - timer_start < 340){
        readSensors(); 
        if(middleSensor == 0){
            // go straight
            leftMotor.go(MotorPower);
            rightMotor.go(-MotorPower);
        }   
        else{
            if(leftSensor == 0){
                //turn left
                leftMotor.go(MotorPower/5);
                rightMotor.go(-MotorPower);
            }
            else{
                if(rightSensor == 0){
                    //turn right
                    leftMotor.go(MotorPower);
                    rightMotor.go(-MotorPower/5);
                }
                else{
                    // black line is between sensors
                    leftMotor.go(MotorPower);
                    rightMotor.go(-MotorPower);
                }
            }
        }
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
    while(millis()-starting_time < target_coordinate.wait * 100){
        leftMotor.go(0);
        rightMotor.go(0);
    }
}

// only used for turning to start position
// because there can be situation when robot is turned into position where is no line to stop him turning
void turn_in_place_90(turning_direction dir){
    int turningTime = 720;
    if(dir == left){
        unsigned long timer_start = millis();
                while(millis() - timer_start < turningTime){
                  leftMotor.go(-MotorPower);
                  rightMotor.go(-MotorPower);
                }
    }
    if(dir == right){
        unsigned long timer_start = millis();
                while(millis() - timer_start < turningTime){
                  leftMotor.go(MotorPower);
                  rightMotor.go(MotorPower);
                }
    }
    leftMotor.go(0);
    rightMotor.go(0);
}


void go_to_start_position(){
    go_to_coordinate(start_position);
    // default positioning
    switch(current_orientation){
        case 'N':
            switch(startingOrientation){
                case 'N':
                    break;
                case 'E':
                    turn_in_place_90(right);
                    break;
                case 'W':
                    turn_in_place_90(left);
                    break;
                case 'S':
                    if(get_current_horizontal_position() == 'A'){
                        turn_in_place_90(right);
                        turn_in_place_90(right);
                    }
                    else{
                        turn_in_place_90(left);
                        turn_in_place_90(left);
                    }
                    break;
            }
            break;
        case 'S':
            switch(startingOrientation){
                case 'S':
                    break;
                case 'W':
                    turn_in_place_90(right);
                    break;
                case 'E':
                    turn_in_place_90(left);
                    break;
                case 'N':
                    if(get_current_horizontal_position() == 'A'){
                        turn_in_place_90(left);
                        turn_in_place_90(left);
                    }
                    else{
                        turn_in_place_90(right);
                        turn_in_place_90(right);
                    }
                    break;
            }
            break;
        case 'W':
            switch(startingOrientation){
                case 'W':
                    break;
                case 'S':
                    turn_in_place_90(left);
                    break;
                case 'N':
                    turn_in_place_90(right);
                    break;
                case 'E':
                    if(get_current_vertical_position() == '1'){
                        turn_in_place_90(right);
                        turn_in_place_90(right);
                    }
                    else{
                        turn_in_place_90(left);
                        turn_in_place_90(left);
                    }
                    break;
            }
            break;
        case 'E':
            switch(startingOrientation){
                case 'E':
                    break;
                case 'S':
                    turn_in_place_90(right);
                    break;
                case 'N':
                    turn_in_place_90(left);
                    break;
                case 'W':
                    if(get_current_vertical_position() == '1'){
                        turn_in_place_90(left);
                        turn_in_place_90(left);
                    }
                    else{
                        turn_in_place_90(right);
                        turn_in_place_90(right);
                    }
                    break;
            }
            break;
    }
    current_orientation = startingOrientation;
}

// go multiple steps straight
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

char get_start_horizontal_position(){
    if(start_position.first >= 'A' && start_position.first <= 'Z'){
        return start_position.first;
    }
    else{
        return start_position.second;
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

char get_start_vertical_position(){
    if(start_position.first >= '1' && start_position.first <= '9'){
        return start_position.first;
    }
    else{
        return start_position.second;
    }
}

// debug function for printing out coordinate
void PrintCoordinate(coordinate coord){
  Serial.print(coord.first);
  Serial.print(' ');
  Serial.print(coord.second);
  Serial.print(' ');
  Serial.print(coord.wait);
  Serial.println(' ');
}

// debug function
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

/**
 * first char could be character or number
 * second char is the other one
 * third char is direction
 */
boolean validateStartingCoordinate(char token []){
    // we are directly checking if input is valid.
    // ascii:
    // A - 65
    // I - 73
    // a - 97
    // i - 105
    // 1 - 49
    // 9 - 57
    boolean isFirstLetter=true;

    if((token[0]>='A' && token[0]<='I') || (token[0]>='a' && token[0]<='i')) {
        isFirstLetter=true;

    }
    else if(token[0]>='1' && token[0]<='9'){
        isFirstLetter=false;
    }
    else{
        return false;
    }


    if(isFirstLetter){ // then second must be number
        if(!(token[1]>='1' && token[1]<='9')){
            return false;
        }
    }
    else{
        if(!((token[1]>='A' && token[1]<='I') || (token[1]>='a' && token[1]<='i'))) {
            return false;
        }

    }

    //  check for direction, ascii values of  N n, E e, S s, W w
    if ( token[2]=='N' || token[2]=='n' || token[2]=='E' || token[2]=='e' || token[2]=='S' || token[2]=='s' || token[2]=='W' || token[2]=='w' ) {
        return true;
    }
    else{
        return false;
    }
}
/**
 * Function that validates coordinates.
 * @param token
 * @return
 */
boolean validateCoordinate(char token []){
    boolean isFirstLetter=true;

    if((token[0]>='A' && token[0]<='I') || (token[0]>='a' && token[0]<='i')) {
        isFirstLetter=true;

    }
    else if(token[0]>='1' && token[0]<='9'){
        isFirstLetter=false;
    }
    else{
        return false;
    }

    if(isFirstLetter){ // then second must be number
        if(!(token[1]>='1' && token[1]<='9')){
            return false;
        }
    }
    else{               // then second must be character
        if(!((token[1]>='A' && token[1]<='I') || (token[1]>='a' && token[1]<='i'))) {
            return false;
        }

    }
    return true;
}
/**
 *
 * @param token array of chars, first should be T or t, and then digits
 * @return
 */
boolean validateTime(char token []){
    if(!(token[0]=='T' || (token[0]=='t' ))) {
        return false;
    }

    boolean isNumber=true;

    for(int i=1;i<6;i++){
        if(token[i]=='X'){
            break;
        }
        else{
            if(!((token[i]>='0' && token[i]<='9'))){
                isNumber=false;
                break;
            }
        }
    }
    return isNumber;
}
/**
 * Can contain:
 * starting position:      A1N
 * position without time:  C4 or 4C
 * time:                   T350
 */
boolean validateInputToken(char token[], parsing_input parsing_state){
    boolean isValid=false;

   
    switch(parsing_state) {
        case reading_starting_coordinate_state:
            Serial.println("State: reading_starting_coordinate_state Token: "+String(token));
            isValid=validateStartingCoordinate(token);
            break;
        case reading_coordinate_state:
            Serial.println("State: reading_coordinate_state Token: "+String(token));
            isValid=validateCoordinate(token);
            break;
        case reading_time_state:
            Serial.println("State: reading_time_state Token: "+String(token));
            isValid=validateTime(token);
            break;
        case malformed_input_state:
            Serial.println("State: malformed_input_state Token: "+String(token));
            //this should not occur
            break;
    }

    return isValid;
}
/**
Uppercases char if it is lower case letter. If number it lets it be.
 */
char upperCase(char c){
    if((c>='a' && c<='z')) {
        return  (char)(c-32);

    }
    else{
        return c;
    }
}

boolean handleSerial() {
    boolean ignoreWhiteSpacesAtStart=true;
    char token[10];
    byte coordinateNumber=0;
    byte counter=0;


    Serial.println("Started parsing input!");

    while (Serial.available() > 0) { 
        Serial.println(counter);
        char incomingCharacter = Serial.read();
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

                        coordinate start_position_new;
                        start_position_new.wait=0;
                        
                        token[0] = upperCase(token[0]); 
                        start_position_new.first= token[0];
                        
                        token[1] = upperCase(token[1]); 
                        start_position_new.second=  token[1];
                                                
                        char starting_orientation_new = (char)( token[2]-32);;
                        
                        pointer_at_current_custom_choreography_byte += EEPROM_write(pointer_at_current_custom_choreography_byte, starting_orientation_new);
                        
                        pointer_at_current_custom_choreography_byte += EEPROM_write(pointer_at_current_custom_choreography_byte, start_position_new);

                        Serial.print(start_position_new.first); Serial.print(" ");
                        Serial.print(start_position_new.second); Serial.print(" ");
                        Serial.print(starting_orientation_new);  Serial.println();
                        

                        parsing_state=reading_coordinate_state;
                        ignoreWhiteSpacesAtStart=true;
                        counter=0;

                        EEPROM_write(0,true);
                        
                        memset(token, 'X', sizeof(token));
                        
                        //TODO: clear array
//                        token= new char[10];

                    }
                    else{
                        parsing_state=malformed_input_state;
                        Serial.println("Malformed input! Restart robot to input new custom choreography or press button to start default choreography."); 
                        EEPROM_write(0,false);
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
                                               
                        token[0] = upperCase(token[0]); 
                        new_coordinate.first = token[0];
                        token[1] = upperCase(token[1]); 
                        new_coordinate.second = token[1];

                        parsing_state=reading_time_state;
                        ignoreWhiteSpacesAtStart=true;
                        counter=0;
//                            memset(token, 'X', sizeof(token));
                        //TODO: clear array
//                        token= new char[10];
                        memset(token, 'X', sizeof(token));
                    }
                    else{
                        parsing_state=malformed_input_state;
                        Serial.println("Malformed input! Restart robot to input new custom choreography or press button to start default choreography."); 
                        EEPROM_write(0,false);
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
                    // TODO: ako vie validate aky je dlhy token array?
                    boolean validCoordinate = validateInputToken(token, parsing_state);
                    Serial.print("is validTime?"); Serial.println(validCoordinate);
                    if(validCoordinate ){
                        // token looks like T350 or T1250
                        int i;
                        String s="";
                        for(i=1;i<6;i++){
                            if(token[i]=='X'){
                                break;
                            }
                            else{
                                s+=token[i];
                            }
                        }
                        
                        unsigned int waitTime= (unsigned int) s.toInt();
                        new_coordinate.wait = waitTime;

                        Serial.print(new_coordinate.first); Serial.print(" ");
                        Serial.print(new_coordinate.second); Serial.print(" ");
                        Serial.print(new_coordinate.wait);  Serial.println();
          

                        pointer_at_current_custom_choreography_byte += EEPROM_write(pointer_at_current_custom_choreography_byte, new_coordinate);
                        
                        EEPROM_write(pointer_at_current_custom_choreography_byte,eoi_mark);
                       

                        parsing_state=reading_coordinate_state;
                        ignoreWhiteSpacesAtStart=true;
                        counter=0;
//                        coordinateNumber++;
                        

//                        memset(token, 'X', sizeof(token));
                        memset(token, 'X', sizeof(token));
//                        token= new char[10];
                    }
                    else{
                        parsing_state=malformed_input_state;
                        Serial.println("Malformed input! Restart robot to input new custom choreography or press button to start default choreography."); 
                        EEPROM_write(0,false);
                        return false;
                    }
                }



                break;
            case malformed_input_state:
                Serial.println("Malformed input! Restart robot to input new custom choreography or press button to start default choreography.");                
                break;
        }
    }
    Serial.println("Ended parsing input!");
    
    
    return true;
}



// if valid (zero byte set to 1), return byte 512 as addrss of loaded choreography else return byte 1 as address of default choreography
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

// debug function
void turn_light_on(int timeInMs){
    unsigned int start = millis();
    while((millis()-start) < timeInMs){
        digitalWrite(11,1);
    }
    digitalWrite(11,0);
}

// expects already written choreography in EEPROM 
void start_dancing(){
    int reading_byte = select_choreography();
    reading_byte += EEPROM_read(reading_byte, startingOrientation);
    current_orientation = startingOrientation;
    Serial.println("reading starting orientation");
    Serial.println(startingOrientation);
    coordinate current_coordinate = {'a','a',0};
    // read starting position (1 = default, 512 = custom)
    reading_byte += EEPROM_read(reading_byte, current_coordinate);
    start_position = current_coordinate;
    Serial.println("start position...");
    PrintCoordinate(start_position);
    current_position= current_coordinate;
    reading_byte += EEPROM_read(reading_byte, current_coordinate);
    // read dance
    while(current_coordinate.first != eoi_mark.first && current_coordinate.second != eoi_mark.second){
        Serial.println(" strart roling....");
        go_to_coordinate(current_coordinate);
        Serial.println("reading coordinate...");
        reading_byte += EEPROM_read(reading_byte, current_coordinate);
        PrintCoordinate(current_coordinate);        
    }
    leftMotor.go(0);
    rightMotor.go(0);
}

// main loop
void loop() {
    switch(robot_state){      
      case waiting_for_start_state:
          Serial.println("waiting_for_start_state");
          
          handleSerial();
          if(digitalRead(2)==0){
            robot_state=doing_choreography_state;
          }
          break;
      case doing_choreography_state:
          Serial.println("doing_choreography_state");
          starting_time=millis();
          start_dancing();
          robot_state=choreography_done_state;
          break;
      case choreography_done_state:
          Serial.println("choreography_done_state");
          
          if(digitalRead(2)==0){
            robot_state=returning_to_start_state;
            go_to_start_position();
            robot_state=waiting_for_start_state;
          }
          break;
      case returning_to_start_state:
          Serial.println("returning_to_start_state");
          break;
    }
//   delay(2000);
}
