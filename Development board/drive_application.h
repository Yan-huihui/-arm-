#ifndef DRIVE_APPLICATION_H
#define DRIVE_APPLICATION_H

int IC_TTY_addToLibIc(void);
int IC_TTY_Option_Set(int fd,int baud_rate,int data_bits,char parity,int stop_bits);
int IC_TTY_Init(const char* pTtyName,int nSpeed,int nBits,char nEvent,int nStop);
int IC_TTY_Destroy(int nComFd);
void* send_th(void* arg);
void* recv_th(void* arg);

#endif // DRIVE_APPLICATION_H
