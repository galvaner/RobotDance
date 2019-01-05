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
// initial choreography starts at 1 byte in EEPROM
// loaded choreography starts at eeprom 512 byte in EEPROM
void SaveInitialChoreographyToEEPROM(){
    int number_of_default_instructions = 8;
    char startingOrientation = 'W';
    coordinate startingPosition = {'1', 'A', 0};
    coordinate instructions[number_of_default_instructions] = {
        {'B','3', 100},
        {'1','C', 200},
        {'3','C', 300},
        {'C','2', 400},
        {'B','3', 500},
        {'A','3', 600},
        {'1','A', 700},
        {'3','C', 800}
      };
    int writing_byte = 0;
    bool read_from_custom = false;
    writing_byte += EEPROM_write(writing_byte, read_from_custom);
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
