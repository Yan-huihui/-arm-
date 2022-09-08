#ifndef DATA_PROCESSING_H
#define DATA_PROCESSING_H

#include "linkque.h"
int get_oil(int fd,SENSOR_DATA *data);
int get_temper(int fd,SENSOR_DATA *data);
int get_gps(int fd,SENSOR_DATA *data);


void oil_data_process(char *buff);
void temper_data_process(char *buff, int adc);
int gps_data_process(char *buff);
#endif // DATA_PROCESSING_H
