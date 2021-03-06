/**
 * Marlin 3D Printer Firmware
 * Copyright (C) 2016 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (C) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/**
 * emergency_parser.h - Intercept special commands directly in the serial stream
 */

#ifndef _EMERGENCY_PARSER_H_
#define _EMERGENCY_PARSER_H_

// External references
extern volatile bool wait_for_user, wait_for_heatup;
void quickstop_stepper();
void dropSerialbuffer();
void execPauseFromSerial();
void priority_command_detected();
void end_of_print_detected();

class EmergencyParser {

public:

  // Currently looking for: M108, M112, M410
  enum State : char {
    EP_RESET,
    EP_N,
    EP_M,
    EP_M1,
    EP_M10,
    EP_M106,
    EP_M108,
    EP_M11,
    EP_M112,
    EP_M15,
    EP_M156,
    EP_M157,
    EP_M2,
    EP_M22,
    EP_M220,
    EP_M221,
    EP_M4,
    EP_M41,
    EP_M410,
    EP_M5,
    EP_M53,
    EP_M537,
    EP_M54,
    EP_M541,
    EP_M545,
    EP_M6,
    EP_M66,
    EP_M669,
    EP_IGNORE // to '\n'
  };

  static bool has_line_number;
  static bool killed_by_M112;
  static State state;

  EmergencyParser() {}

  __attribute__((always_inline)) inline
  static void update(const uint8_t c) {

    switch (state) {
      case EP_RESET:
        switch (c) {
          case ' ': break;
          case 'N': state = EP_N; has_line_number = true; break;
          case 'M': state = EP_M; has_line_number = false; break;
          default: state  = EP_IGNORE;
        }
        break;

      case EP_N:
        switch (c) {
          case '0': case '1': case '2':
          case '3': case '4': case '5':
          case '6': case '7': case '8':
          case '9': case '-': case ' ': break;
          case 'M': state = EP_M;       break;
          default:  state = EP_IGNORE;
        }
        break;

      case EP_M:
        switch (c) {
          case ' ': break;
          case '1': state = EP_M1;     break;
          case '2': state = EP_M2;     break;
          case '4': state = EP_M4;     break;
          case '5': state = EP_M5;     break;
          case '6': state = EP_M6;     break;
          default: state  = EP_IGNORE;
        }
        break;

      case EP_M1:
        switch (c) {
          case '0': state = EP_M10;    break;
          case '1': state = EP_M11;    break;
          case '5': state = EP_M15;    break;
          default: state  = EP_IGNORE;
        }
        break;

      case EP_M10:
        switch (c) {
          case '6': state = EP_M106;    break;
          case '8': state = EP_M108;    break;
          default: state  = EP_IGNORE;
        }
        break;

      case EP_M11:
        state = (c == '2') ? EP_M112 : EP_IGNORE;
        break;

      case EP_M15:
        switch (c) {
          case '6': state = EP_M156;    break;
          case '7': state = EP_M157;    break;
          default: state  = EP_IGNORE;
        }
        break;

      case EP_M2:
        state = (c == '2') ? EP_M22 : EP_IGNORE;
        break;

      case EP_M22:
        switch (c) {
          case '0': state = EP_M220;    break;
          case '1': state = EP_M221;    break;
          default: state  = EP_IGNORE;
        }
        break;

      case EP_M4:
        state = (c == '1') ? EP_M41 : EP_IGNORE;
        break;

      case EP_M41:
        state = (c == '0') ? EP_M410 : EP_IGNORE;
        break;
		
      case EP_M5:
        switch (c) {
          case '3': state = EP_M53;    break;
          case '4': state = EP_M54;    break;
          default: state  = EP_IGNORE;
        }
        break;

      case EP_M53:
        state = (c == '7') ? EP_M537 : EP_IGNORE;
        break;

      case EP_M54:
        switch (c) {
          case '1': state = EP_M541;    break;
          case '5': state = EP_M545;    break;
          default: state  = EP_IGNORE;
        }
        break;
      
      case EP_M6:
        state = (c == '6') ? EP_M66 : EP_IGNORE;
        break;

      case EP_M66:
        state = (c == '9') ? EP_M669 : EP_IGNORE;
        break;

      case EP_IGNORE:
        if (c == '\n') state = EP_RESET;
        break;

      default:
        if (c == '\n') {
          switch (state) {
            case EP_M108:
              wait_for_user = wait_for_heatup = false;
              break;
            case EP_M112:
              killed_by_M112 = true;
              break;
            case EP_M410:
              quickstop_stepper();
              break;
            case EP_M541:
              dropSerialbuffer();			  			  
              break;
            case EP_M669:
              execPauseFromSerial();
              break;
            #ifdef BCN3D_MOD
              case EP_M106:
              case EP_M156:
              case EP_M157:
              case EP_M220:
              case EP_M221:
              case EP_M537:
                if (!has_line_number) priority_command_detected();
                break;
              case EP_M545:
                end_of_print_detected();
                break;
            #endif
            default:
              break;
          }
          state = EP_RESET;
        }
    }
  }

};

extern EmergencyParser emergency_parser;

#endif // _EMERGENCY_PARSER_H_
