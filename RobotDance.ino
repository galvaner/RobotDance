// TODO: predpokladame ze ked sa strci kabel, zacina custom dance, a pri vytiahnuti kabla sa zapise do eepromky

enum robot_state
{
  waiting_for_start_state,
  doing_choreography_state,
  choreography_done_state,
  returning_to_start_state
};

enum turning_direction
{
  left,
  right
};


typedef struct coordinate{
  char first; //x-coordinate
  char second; //y-coordinate
  unsigned long wait; //when to move next
};

typedef struct choreography{
  char stratringOrientation;
  coordinate startingPosition;
  coordinate choreographyArray [];   // does not contain starting position!
};

coordinate start_position;
coordinate current_position;
//coordinate dance_choreography[];

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

// initial choreography starts at 0 byte in EEPROM
// loaded choreography starts at eeprom.length/2 byte in EEPROM
//void LoadInitialChoreographyToEEPROM(){
//    coordinate instructions[] = {
//        coordinate();
//      }
//}

// TODO: setEPROM, getEprom (zapis a potom nacitaj choreografiu)
// zresetovat eepromku,
//void parse_input_to_coordinate(string input_file){return;}
//void turn_in_place(turning_direction direction, number_of_times){return;}
//void go_straight(number_of_fields) {return;}
//void go_to_coordinate(coordinate initial_coordinate, coordinate target_coordinate){return;}
//void go_to_start_position(){return;}

void setup() {
  // TODO: reset eeprom and write default dance here to eeprom

}


void handleSerial() {
 while (Serial.available() > 0) {
   char incomingCharacter = Serial.read();   
 }
}
robot_state rs = waiting_for_start_state;
// main loop
void loop() {
    switch(rs){
      case waiting_for_start_state:
          handleSerial();
          break;
      case doing_choreography_state:
          break;
      case choreography_done_state:
          break;
      case returning_to_start_state:
          break;         
    }
}
