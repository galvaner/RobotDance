// TODO: predpokladame ze ked sa strci kabel, zacina custom dance, a pri vytiahnuti kabla sa zapise do eepromky

enum robot_state
{
  waiting_for_start_state,
  doing_choreography_state,
  choreography_done_state,
  returning_to_start_state
}

enum turning_direction
{
  left,
  right
}

enum parsing_input{
  start_state,
  reading_starting_position_state,
  reading_position_state,
  reading_time_state,
  malformed_input_state
}

typedef struct coordinate{
    char first; //x-coordinate
    char second; //y-coordinate
//    unsigned long wait; //when to move next
};

typedef struct choreography{
  char stratringOrientation;
  coordinate startingPosition;
  coordinate [] choreographyArray;   // does not contain starting position!
};


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
coordinate [] dance_choreography;

void setup() {
  // TODO: reset eeprom and write default dance here to eeprom

}


/**
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
        char incomingCharacter = serial.read();
        switch(robot_state){
            case parsing_input_state:
                //mozem citat lubovolne whitespacy, ale akonahle bude char, chod do reading reading_starting_position_state
                break;
            case reading_starting_position_state:
                // citaj chary, ale akonahle bude whitespace, zvaliduj position a chod bud do malformed alebo do reading_position_state
                break;
            case reading_position_state:
                // citaj chary, ale akonahle je whitespace zvaliduj, a chod bud do reading_time_stat alebo do  malformed_input_state
                break;
            case reading_time_stat:
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
    switch(robot_state){
      case waiting_for_start_state:
          boolean useCustomChoreography = handleSerial();
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
