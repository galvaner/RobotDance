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
typedef struct coordinate
  {
      int number; //x-coordinate
      char letter; //y-coordinate
      char heading;
      bool is_number_first;
      unsigned long wait; //when to move next
  };

// TODO: setEPROM, getEprom (zapis a potom nacitaj choreografiu)
// zresetovat eepromku,
void parse_input_to_coordinate(string input_file){}
void turn_in_place(turning_direction direction, number_of_times){}
void go_straight(number_of_fields) {}
void go_to_coordinate(coordinate initial_coordinate, coordinate target_coordinate){}
void go_to_start_position()

coordinate start_position;
coordinate current_position;
coordinate[] dance_choreography;

void setup() {
  // TODO: reset eeprom and write default dance here to eeprom

}

void handleSerial() {
 while (Serial.available() > 0) {
   char incomingCharacter = serial.read();
   switch (incomingCharacter) {
     case ‘+’:
      pwmValue = pwmValue + 5;
      If (pwmValue >= pwmMax)
         pwmValue = pwmMax;
      break;
 
     case ‘-’:
      pwmValue = pwmValue - 5;
      If (pwmValue <= 0)
         pwmValue = 0;
      break;
    }
 }
}

void loop() {
    switch(robot_state){
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
