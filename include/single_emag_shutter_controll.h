#include <stdint.h>




int shutter_action_countdown(uint32_t countdown_ms, uint32_t shutterOpenInterval_us, TaskHandle_t* shutter_action_task);
bool is_shutter_open();
bool is_shutter_working();
bool is_shutter_task_running();
bool is_shutter_task_ended();
void open_shutter();
void close_shutter();
