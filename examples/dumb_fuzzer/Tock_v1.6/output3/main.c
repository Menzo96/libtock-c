#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <console.h>
#include <timer.h>

#define SIZE 100
#define TESTS_PER_DRIVER 5

void print_error_code(const char* msg, int ret);
void random_syscalls(void);

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

void random_syscalls(void)
{
  int ret;
  int driver_no, driver_idx_no;
  int command_no, command_idx_no;

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

      ret = command_no ? -command_no : TOCK_ENOSUPPORT;

      print_error_code("command: ", ret);
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
