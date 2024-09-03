/*
 * =====================================
 * ais.h
 * =====================================
 * History:
 *   01/09/24 - Creation
 * =====================================
 */
#ifndef AIS_H
#define AIS_H

#include "types.h"
#include "ais_channels.h"
#include "si4463.h"
#include "si4362.h"
#include "version.h"


#define SI4463_PART_INFO 0x4463
#define SI4362_PART_INFO 0x4362

#define si446x_1 SI4362::si446x1_
#define si446x_2 SI4362::si446x2_

void idle();

void manage_inactivity();

void FlushSerialRequestResend();
void ok_to_send();

void kill();

extern uint8_t marlin_debug_flags;

extern bool Running;
inline bool IsRunning() { return  Running; }
inline bool IsStopped() { return !Running; }

bool enqueue_and_echo_command(const char* cmd, bool say_ok=false); // Add a single command to the end of the buffer. Return false on failure.
void enqueue_and_echo_commands_P(const char * const cmd);          // Set one or more commands to be prioritized over the next Serial/SD command.
void clear_command_queue();

//extern millis_t previous_cmd_ms;
//inline void refresh_cmd_timeout() { previous_cmd_ms = millis(); }



// GCode support for external objects
bool code_seen(char);
int code_value_int();





/*
 * =========================
 * Externals
 * =========================
 */
extern gps_data_t Gps_info_s;
extern StationData Station_data_s;
extern volatile uint32_t Timer_ticks_5_u32;
extern volatile uint32_t Timer_ticks_1_u32;

#endif //AIS_H
