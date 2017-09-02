//
//  ElmoErrorCodes.hpp
//  am2b
//
//  Created by Felix Sygulla on 2016-03-09.
//  Copyright 2015 Chair of Applied Mechanics, TUM
//  https://www.amm.mw.tum.de/
//

#ifndef ELMOERRORCODES_HPP_EA6301B0
#define ELMOERRORCODES_HPP_EA6301B0

#include <string>

namespace ec {

  //! Stores the error codes for Elmo devices
  class ElmoErrorCodes {
  
  public:
    
    //! Returns an error message for the given errorCode (emergency Code) / Elmo Error Code
    static std::string getErrorStringEmergency(const uint16_t& errorCode, const uint8_t ElmoErrorCode) {
      
      switch (errorCode) {
        
        case 0x2340:
        return "Short Circuit";
        
        case 0x3120:
        return "Under-voltage";
        
        case 0x3130:
        return "AC fail, loss of phase";
        
        case 0x3310:
        return "Over-voltage";
        
        case 0x4310:
        return "Temperature: drive overheating";
        
        case 0x5280:
        return "Gantry position error";
        
        case 0x5441:
        return "Motor disabled by INHIBIT or ABORT (limit switches?)";
        
        case 0x5442:
        return "Motor disabled by switch 'additional abort motion'";
        
        case 0x6300:
        return "RPDO failed, " + getElmoErrorCodeDescription(ElmoErrorCode);
        
        case 0x7121:
        return "Motor stuck";
        
        case 0x7300:
        return "Feedback error (Possible EnDat / Encoder failure)";
        
        case 0x7381:
        return "Two digital Hall sensors were changed at the same time";
        
        case 0x7382:
        return "Commutation process fail during motor on";
        
        case 0x8110:
        return "CAN message lost (corrupted or overrun), " + getElmoErrorCodeDescription(ElmoErrorCode);
        
        case 0x8130:
        return "Heartbeat event, " + getElmoErrorCodeDescription(ElmoErrorCode);
        
        case 0x8140:
        return "Recovered from bus off, " + getElmoErrorCodeDescription(ElmoErrorCode);
        
        case 0x8210:
        return "Attempt to access a non-configured RPDO";
        
        case 0x8311:
        return "The peak current has been exceeded. Possible reasons are drive malfunction or bad tuning of the current controller";
        
        case 0x8480:
        return "Speed tracking error exceeded speed error limit ER[2]";
        
        case 0x8481:
        return "Speed limit exceeded (LL[2] or HL[2])";
        
        case 0x8611:
        return "Position tracking error exceeded position error limit ER[3]";
        
        case 0x8680:
        return "Position limit exceeded (LL[3] or HL[3])";
        
        case 0xFF02:
        return "Miscellaneous error, " + getElmoErrorCodeDescription(ElmoErrorCode);
        
        case 0xFF10:
        return "Failed to start motor, " + getElmoErrorCodeDescription(ElmoErrorCode);
        
        case 0xFF20:
        return "Safety Torque Off in use";
        
        case 0xFF40:
        return "Gantry Slave Disabled";
        
        default:
        return "Unknown Emergency Error " + toString((unsigned int) errorCode) + ", " + getElmoErrorCodeDescription(ElmoErrorCode);
        
      }
      
      
    }
    
    //! Returns a description for the Elmo Error Code
    static std::string getElmoErrorCodeDescription(const uint8_t& ElmoErrorCode) {
      
      switch (ElmoErrorCode) {
        
        case 2: return "Bad Command";
      
        case 3: return "Bad Index";
        
        case 4: return "PAL does not support this sensor";
        
        case 7: return "Mode cannot be started - bad initialization data";
        
        case 9: return "CAN message HW buffer was overrun";
        
        case 10: return "Cannot be used by PDO";

        case 11: return "Cannot write to flash memory";

        case 13: return "Cannot reset communication -UART is bury";

        case 16: return "Array '[ ]' is expected, or empty expression in arry";

        case 17: return "Format of UL command is not valid  - check the command definition";

        case 19: return "Command syntax error";

        case 20: return "Bad Set Point sending order";

        case 21: return "Operand Out of Range";

        case 23: return "Command cannot be assigned";

        case 26: return "Profiler mode not supported in this unit mode (UM)";

        case 27: return "Bad ECAM setting";

        case 28: return "Out Of Limit Range";

        case 33: return "Bad sensor setting";

        case 34: return "There is a conflict with another command";

        case 35: return "Max bus voltage (BV) or max current (MC) is not valid";

        case 36: return "Commutation method (CA[17]) or commutation table does not fit to sensor";

        case 37: return "Two Or More Hall sensors are defined to the same place";

        case 38: return "PORT C setting is incorrect";

        case 40: return "In Wizard Experiment!";

        case 42: return "No Such Label";

        case 45: return "An Attempt to read a write only command";

        case 47: return "Program does not exist or not Compiled";

        case 48: return "Motor cold not start - fault reason in CD";

        case 51: return "Inhibit OR Abort inputs are active, Cannot start motor";

        case 54: return "Bad Data Base";

        case 57: return "Motor Must be Off";

        case 58: return "Motor Must be On";

        case 60: return "Bad Unit Mode";

        case 61: return "Data Base Reset";

        case 62: return "Socket change not allowed";

        case 67: return "Recorder Is Busy";

        case 68: return "Required profiler mode is not supported";

        case 69: return "Recorder Usage Error";

        case 70: return "Recorder data Invalid";

        case 71: return "Homing is busy";

        case 74: return "Bad profile database, see 0x2081 for object number (EE[2])";

        case 75: return "Download is in progress";

        case 76: return "Error mapping is not allowed";

        case 78: return "Out of Program Range";

        case 79: return "Sensor setting error";

        case 81: return "Download failed see specific error in EE[3]";

        case 82: return "Program Is Running";

        case 83: return "Command is not permitted in a program.";

        case 85: return "STO is not active";

        case 87: return "Hall sensed with illegal value";

        case 94: return "Not allowed while Error mapping";

        case 97: return "RS232 receive buffer overflow";

        case 98: return "Can not measure current offsets";

        case 99: return "The sensor does not support this command";

        case 100: return "The requested PWM value is not supported";

        case 101: return "Absolute encoder setting problem";

        case 102: return "Output Compare is busy";

        case 103: return "Output Compare Sensor Is Not QUAD Encoder";

        case 104: return "Output Compare Table Length OR Data";

        case 105: return "Speed loop KP out of range";

        case 107: return "Encoder emulation parameter is out of range";

        case 108: return "Encoder emulation in progress";

        case 110: return "Too long number";

        case 122: return "Motion mode is not supported or with initialization conflict";

        case 123: return "Profiler queue is full";

        case 125: return "Personality not loaded";

        case 126: return "User Program failed - variable out of program size";

        case 128: return "Bad variable index in database";

        case 129: return "Variable is not an array";

        case 130: return "Variable name does not exist";

        case 131: return "Cannot record local variable";

        case 132: return "Variable is an array";

        case 133: return "Number of function input arguments is not as expected";

        case 134: return "Cannot run local label/function with the XQ command";

        case 135: return "Frequency identification failed";

        case 136: return "Not a number";

        case 138: return "Position Interpolation buffer underflow";

        case 139: return "The number of break points exceeds maximal number";

        case 140: return "An attempt to set/clear break point at the not relevant line ";

        case 142: return "Checksum of data is not correct";

        case 144: return "Numeric Stack underflow";

        case 145: return "Numeric stack overflow";

        case 147: return "Executable command within math expression";

        case 148: return "Nothing in the expression";

        case 151: return "Parentheses mismatch";

        case 152: return "Bad operand type";

        case 153: return "Overflow in a numeric operator";

        case 154: return "Address is out of data memory segment";

        case 155: return "Beyond stack range";

        case 156: return "Bad op-code";

        case 158: return "Out of flash memory range";

        case 159: return "Flash memory verification error";

        case 161: return "Program is not halted";

        case 163: return "Not enough space in program data segment";

        case 165: return "An attempt to access flash memory while busy";

        case 166: return "Out Of Modulo Range";

        case 168: return "Speed too large to start motor";

        case 169: return "Time out using peripheral.(overflow or busy)";

        case 170: return "Cannot erase sector in flash memory";

        case 171: return "Cannot read from flash memory";
        
        case 172: return "Cannot write to flash memory";
        
        case 173: return "Executable area of program is too large";
        
        case 174: return "Program has not been loaded";
        
        case 175: return "Cannot write program checksum - clear program (CP)";
        
        case 176: return "User code, variables and functions are too large";
        
        case 177: return "Capture/Compare conversion error in analog encoder. Require known quad location";
        
        case 178: return "CAN bus off";
        
        case 179: return "Consumer HD event";
        
        case 180: return "DF is not supported in this communication type";
        
        case 181: return "Writing to flash program area failed";
        
        case 182: return "PAL burn in process or no PAL is burned";
        
        case 184: return "Capture option already used by other operation";
        
        case 185: return "This element may be modified only when interpolation is not active";
        
        case 186: return "Interpolation queue is full";
        
        case 187: return "Incorrect interpolation sub-mode";
        
        case 188: return "Gantry slave is disabled";
        
        case 189: return "CAN message was lost, software buffer overflow";
        
        case 200: return "Main Feedback error, refer to EE[1]";
        
        case 201: return "Commutation sequence failed";
        
        case 202: return "Encoder - Hall sensor mismatch, refer to XP[7]";
        
        case 203: return "Current limit was exceeded";
        
        case 204: return "External inhibit input detected";
        
        case 205: return "AC Fail: loss of phase";
        
        case 206: return "Digital Hall run too fast or disconnected";
        
        case 207: return "Speed error limit exceeded, refer to ER[2]";
        
        case 208: return "Position error limit exceeded value of ER[3]";
        
        case 209: return "Cannot start motor because of bad database refer to: CD";
        
        case 210: return "Bad ECAM table";
        
        case 216: return "Cannot find zero position without DHalls";
        
        case 217: return "Over Speed violation, refer to HL[2]";
        
        case 221: return "Motor stuck";
        
        case 222: return "Out of position limits, refer to HL[3] or LL[3]";
        
        case 223: return "Numerical overflow";
        
        case 224: return "Gantry slave is not enabled";
        
        case 229: return "Cannot start motor because of internal problem";
        
        case 233: return "Under voltage protection";
        
        case 235: return "Overvoltage protection";
        
        case 237: return "Safety switch";
        
        case 241: return "Short protection";
        
        case 243: return "Over temperature protection";
        
        case 245: return "Additional inhibit input";
        
        default:
        return "Unknown Elmo Error Code " + toString((unsigned int) ElmoErrorCode);
        
      }

    }
    
  private:
    
    //! Convert integer to string
    static std::string toString(unsigned int value) {
      
      static char m_buf[15];
      
      snprintf(m_buf, 15, "%u", value);
      return std::string(m_buf);
      
    }
  
  }; 


}


#endif /* end of include guard: ELMOERRORCODES_HPP_EA6301B0 */
