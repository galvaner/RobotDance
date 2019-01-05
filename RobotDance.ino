// TODO: predpokladame ze ked sa strci kabel, zacina custom dance, a pri vytiahnuti kabla sa zapise do eepromky

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

// TODO: setEPROM, getEprom (zapis a potom nacitaj choreografiu)
// zresetovat eepromku,
//void parse_input_to_coordinate(string input_file){}
//void turn_in_place(turning_direction direction, number_of_times){}
//void go_straight(number_of_fields) {}
//void go_to_coordinate(coordinate initial_coordinate, coordinate target_coordinate){}
//void go_to_start_position(){}

parsing_input parsing_input_state;
char stratringOrientation;
coordinate start_position;
coordinate current_position;
coordinate dance_choreography [100];

void setup() {
  // TODO: reset eeprom and write default dance here to eeprom
  Serial.begin(115200);
  ReadDefaultChoreographyFromEEPROM();
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
                        stratringOrientation = token[2]; 

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

// main loop
void loop() {
     //SaveInitialChoreographyToEEPROM();
     //ReadDefaultChoreographyFromEEPROM();
     //Serial.println("ahoj");

  
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
