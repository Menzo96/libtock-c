#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <console.h>
#include <timer.h>
#include <adc.h>

#include <time.h>

#define SIZE 100
#define SYS_ALLOW 0
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
    case TOCK_SUCCESS:
      strcat(str, "Success\r\n");
      break;

    case TOCK_FAIL:
      strcat(str, "Generic failure condition\r\n");
      break;

    case TOCK_EBUSY:
      strcat(str, "Underlying system is busy; retry\r\n");
      break;

    case TOCK_EALREADY:
      strcat(str, "The state requested is already set\r\n");
      break;

    case TOCK_EOFF:
      strcat(str, "The component is powered down\r\n");
      break;

    case TOCK_ERESERVE:
      strcat(str, "Reservation required before use\r\n");
      break;

    case TOCK_EINVAL:
      strcat(str, "An invalid parameter was passed\r\n");
      break;

    case TOCK_ESIZE:
      strcat(str, "Parameter passed was too large\r\n");
      break;

    case TOCK_ECANCEL:
      strcat(str, "Operation cancelled by a call\r\n");
      break;

    case TOCK_ENOMEM:
      strcat(str, "Memory required not available\r\n");
      break;

    case TOCK_ENOSUPPORT:
      strcat(str, "Operation or command is unsupported\r\n");
      break;

    case TOCK_ENODEVICE:
      strcat(str, "Device does not exist\r\n");
      break;

    case TOCK_EUNINSTALLED:
      strcat(str, "Device is not physically installed\r\n");
      break;

    case TOCK_ENOACK:
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
  int ret;
  int driver_no;
  int command_no, allow_no, subscribe_no, memop_no;
  int sys_call;

  int len = 2000;
  char buf[] = "Hello world!\r\n";
  void *cb, *userdata;

  int adc_channel = 0;
  uint16_t adc_value = 0;

  int driver_idx[] = {  0x00000, 0x00001, 0x00002, 0x00003, 0x00005, 0x00004,
                        0x40001, 0x50000, 0x60000, 0x60004, 0x60006, 0x70006
                      };

  int no_drivers = sizeof driver_idx / sizeof driver_idx[0] ;

  if(adc_sample_sync (adc_channel, &adc_value) != TOCK_SUCCESS)
    return;

  printf("Seed: %d\r\n", adc_value);
  srand((unsigned)adc_value);

  while(1)
  {
    driver_no = driver_idx[rand() % no_drivers];
    printf("Driver number: %x\r\n", driver_no);

    for(int i = 0; i < TESTS_PER_DRIVER; i ++)
    {
      sys_call = rand() % 3;

      switch(sys_call)
      {
        case SYS_ALLOW:
          allow_no = rand() % 20;
          ret = allow(driver_no, allow_no, buf, len);
          print_error_code("allow: ", ret);
          printf("Allow number: %d\r\n", allow_no);

          delay_ms(100);
          break;

        case SYS_SUBSCRIBE:
          subscribe_no = rand() % 20;
          ret = subscribe(driver_no, subscribe_no, cb, userdata);
          print_error_code("subscribe: ", ret);
          printf("Subscribe number: %d\r\n", subscribe_no);

          delay_ms(100);
          break;

        case SYS_COMMAND:
          command_no = rand() % 20;
          ret = command(driver_no, command_no, len, 0);
          print_error_code("command: ", ret);
          printf("Command number: %d\r\n", command_no);

          delay_ms(100);
          break;

        default:
          break;

      }
      printf("\r\n");
    }

  }
}

int main(void)
{
  random_syscalls();
  return 0;
}
