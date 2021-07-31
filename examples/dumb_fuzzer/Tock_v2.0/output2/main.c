#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <console.h>
#include <timer.h>

#define SIZE 100
#define SYS_ALLOW_READWRITE 0
#define SYS_SUBSCRIBE 1
#define SYS_COMMAND 2
#define SYS_MEMOP 3

#define TESTS_PER_DRIVER 5

void print_error_code(const char* msg, int ret);
void random_syscalls(void);

/*
  This function interprets the return values passed in ret and fills the buffer
  msg with the corresponding return value string.
*/
void print_error_code(const char* msg, int ret)
{
  char *str, *aux;

  str = malloc(SIZE);
  aux = malloc(SIZE);
  strcpy(str, msg);

  switch(ret)
  {
    case TOCK_STATUSCODE_SUCCESS:
      strcat(str, "Success\r\n");
      break;

    case TOCK_STATUSCODE_FAIL:
      strcat(str, "Generic failure condition\r\n");
      break;

    case TOCK_STATUSCODE_BUSY:
      strcat(str, "Underlying system is busy; retry\r\n");
      break;

    case TOCK_STATUSCODE_ALREADY:
      strcat(str, "The state requested is already set\r\n");
      break;

    case TOCK_STATUSCODE_OFF:
      strcat(str, "The component is powered down\r\n");
      break;

    case TOCK_STATUSCODE_RESERVE:
      strcat(str, "Reservation required before use\r\n");
      break;

    case TOCK_STATUSCODE_INVAL:
      strcat(str, "An invalid parameter was passed\r\n");
      break;

    case TOCK_STATUSCODE_SIZE:
      strcat(str, "Parameter passed was too large\r\n");
      break;

    case TOCK_STATUSCODE_CANCEL:
      strcat(str, "Operation cancelled by a call\r\n");
      break;

    case TOCK_STATUSCODE_NOMEM:
      strcat(str, "Memory required not available\r\n");
      break;

    case TOCK_STATUSCODE_NOSUPPORT:
      strcat(str, "Operation or command is unsupported\r\n");
      break;

    case TOCK_STATUSCODE_NODEVICE:
      strcat(str, "Device does not exist\r\n");
      break;

    case TOCK_STATUSCODE_UNINSTALLED:
      strcat(str, "Device is not physically installed\r\n");
      break;

    case TOCK_STATUSCODE_NOACK:
      strcat(str, "Packet transmission not acknowledged\r\n");
      break;

    default:
      snprintf(aux, 50, "Other error code: %d\r\n", ret);
      strcat(str, aux);
  }

  printf("%s", str);
  free(str);

}

/*
  This function "fuzzes" the TockOS syscall interface.
  It uses random values seeded from an ADC read to cycle through a few TockOS syscalls
  (allow_readwrite, subscribe, command)
  The parameters passed to the syscalls are random numbers or local buffers/uninitialised pointers

  This function also logs the the parameters and the syscalls' return codes. 
*/
void random_syscalls(void)
{
  int driver_no, driver_idx_no;
  int command_no, command_idx_no;

  int sys_call;
  allow_rw_return_t allow_ret;
  subscribe_return_t subscribe_ret;
  syscall_return_t command_ret;

  int len = 2000;
  char buf[] = "Hello world!\r\n";
  void *cb, *userdata;

  int driver_idx[] = {  0x00000, 0x00001, 0x00002, 0x00003, 0x00005, 0x00004,
                        0x40001, 0x50000, 0x60000, 0x60004, 0x60006, 0x70006
                      };

  int no_drivers = sizeof driver_idx / sizeof driver_idx[0] ;

  driver_idx_no = 1;
  command_idx_no = 3;

  while(1)
  {
    driver_idx_no++;
    driver_idx_no = driver_idx_no * command_idx_no;
    driver_no = driver_idx[driver_idx_no % no_drivers];

    printf("Driver number: %x\r\n", driver_no);

    for(int i = 0; i < TESTS_PER_DRIVER; i ++)
    {
      command_idx_no++;
      command_no = (driver_idx_no * command_idx_no ) % 14;

      command_ret = command(driver_no, command_no, len, 0);
      print_error_code("command: ", command_ret.type);
      printf("Command number: %d\r\n", command_no);

      delay_ms(100);
      
      printf("\r\n");
    }

  }
}

int main(void)
{

  random_syscalls();
  return 0;
}
