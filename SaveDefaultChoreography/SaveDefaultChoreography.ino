#include <EEPROM.h>
#include <Arduino.h>  // for type definitions

typedef struct coordinate{
    char first; //x-coordinate
    char second; //y-coordinate
    unsigned int wait; //when to move next
};

void PrintCoordinate(coordinate coord){
  Serial.print(coord.first);
  Serial.print(' ');
  Serial.print(coord.second);
  Serial.print(' ');
  Serial.print(coord.wait);
  Serial.println(' ');
}

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

// end of instruction mark
coordinate eoi_mark = {'q', 'q', 0};
// initial choreography starts at 0 byte in EEPROM
// loaded choreography starts at eeprom.length/2 byte in EEPROM
void SaveInitialChoreographyToEEPROM(){
    int number_of_default_instructions = 2;
    char startingOrientation = 'N';
    coordinate startingPosition = {'C', '0', 0};
    coordinate instructions[number_of_default_instructions] = {
        {'A', '1', 125},
        {'2', 'B', 269}
      };
    int writing_byte = 1;
    writing_byte += EEPROM_write(writing_byte, startingOrientation);
    Serial.print(writing_byte);
    Serial.println(startingOrientation);
    writing_byte += EEPROM_write(writing_byte, startingPosition);
    Serial.print(writing_byte);
    PrintCoordinate(startingPosition);
    for(int i=0; i<number_of_default_instructions; i++){
        writing_byte += EEPROM_write(writing_byte, instructions[i]);
        Serial.print(writing_byte);
        PrintCoordinate(instructions[i]);
    }
    // write end of instructions mark
    writing_byte += EEPROM_write(writing_byte, eoi_mark);
}

void setup() {
  Serial.begin(115200);
  SaveInitialChoreographyToEEPROM();
}

void loop(){
}
