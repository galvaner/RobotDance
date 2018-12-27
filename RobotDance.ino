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
  start_state,
  reading_starting_position_state,
  reading_position_state,
  reading_time_state,
  malformed_input_state
};

typedef struct coordinate{
    char first; //x-coordinate
    char second; //y-coordinate
    unsigned int wait; //when to move next
};

typedef struct choreography{
  char stratringOrientation;
  coordinate startingPosition;
  coordinate choreographyArray [];   // does not contain starting position!
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
//boolean validateInputToken(char [] chars){
//   return true;
//}
boolean handleSerial() {
    while (Serial.available() > 0) {
//        [] cha
        char incomingCharacter = Serial.read();
        parsing_input pis = start_state;
        switch(pis){
            case start_state:
                //mozem citat lubovolne whitespacy, ale akonahle bude char, chod do reading reading_starting_position_state
                break;
            case reading_starting_position_state:
                // citaj chary, ale akonahle bude whitespace, zvaliduj position a chod bud do malformed alebo do reading_position_state
                break;
            case reading_position_state:
                // citaj chary, ale akonahle je whitespace zvaliduj, a chod bud do reading_time_stat alebo do  malformed_input_state
                break;
            case reading_time_state:
                // citaj chary, ale akonahle je whitespace zvaliduj, a chod do reading_position_state alebo malformed_input_state
                break;
            case malformed_input_state:
                // prestan citat zo serial liny
                break;
        }
   }


}

// main loop
void loop() {
     //SaveInitialChoreographyToEEPROM();
     //ReadDefaultChoreographyFromEEPROM();
     //Serial.println("ahoj");

  
    robot_state rs = waiting_for_start_state;
    switch(rs){
      case waiting_for_start_state:
          boolean useCustomChoreography; // = handleSerial();
          if(useCustomChoreography){
            // do dance with custom choreography
          }
          else{
            // do dance with default choreography
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
