//https://www.ics.com/blog/gpio-programming-exploring-libgpiod-library
//https://github.com/starnight/libgpiod-example/blob/master/libgpiod-led/main.c
//https://github.com/starnight/libgpiod-example/blob/master/libgpiod-event/main.c

#include <gpiod.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>     // for exit()
#include <string.h>     // for strcmp()
#include <signal.h>     // for signal()
#include <sys/stat.h>   // for umask()


volatile sig_atomic_t handler_exit = 0;
// Signal handler function
void signal_handler(int signo)
{
    fprintf(stderr, "Caught signal %d, exiting\n", signo);
    handler_exit = 1; // set the flag to exit and remove the file
}

// Function to run the application as daemon
void daemonize()
{
    pid_t pid = fork(); // create a child process
    if (pid < 0)
    {
        exit(EXIT_FAILURE); // fork failed
    }
    if (pid > 0)
    {
        exit(EXIT_SUCCESS); // Parent exits
    }
    if (setsid() < 0)
    {
        exit(EXIT_FAILURE); // creates a new session
    }
    freopen("/dev/null", "r", stdin); // redirects the standard input/output/error to null directory to discard
    freopen("/dev/null", "w", stdout);
    freopen("/dev/null", "w", stderr);
    umask(0);   // file mode creation mask -default file permissions
    chdir("/"); // change to root directory
}



int main(int argc, char **argv)
{
	const char *chipname = "/dev/gpiochip0";
	const char *consumer = "gpio_button";
	unsigned int button_line_num = 4;	// header pin #7
	unsigned int led_press_num = 17;  //header 11
	unsigned int led_test_num = 27;    //header 13
	
	 struct gpiod_line_event event;
	struct gpiod_chip *chip;
	struct gpiod_line *button_line;   //input line to check the button state
	struct gpiod_line *led_press_line; //output line to indicate button press
	struct gpiod_line *led_test_line;   //output line to indicate if server is runnning
	int ret;

// set up the signal handler
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

if (argc > 1 && strcmp(argv[1], "-d") == 0)
    {                // checking if -d arg is passed
        daemonize(); // call daemon function
    }
    
// opens the GPIO chip
	chip = gpiod_chip_open(chipname); //chip open by path
	if (!chip) {
		perror("Open chip failed\n");
		ret=-1;
		goto end;
	}

//opens the  GPIO line 
	button_line = gpiod_chip_get_line(chip, button_line_num);  //for button press
	if (!button_line) {
		perror("Get line failed for button press\n");
		goto close_chip;
	}
	led_press_line = gpiod_chip_get_line(chip, led_press_num);  //led for button press
	if (!led_press_line) {
		perror("Get line failed for the led of button press\n");
		goto close_chip;
	}
        led_test_line = gpiod_chip_get_line(chip, led_test_num);  //led for server run
	if (!led_test_line) {
		perror("Get line failed for the led of server \n");
		goto close_chip;
	}

// request line as input with event detection 
    ret = gpiod_line_request_rising_edge_events(button_line,consumer);
    if (ret < 0) {
        perror("Request line as event input failed");
        goto release_line;
    }

//request line as output
    if ((gpiod_line_request_output(led_press_line, "press", 0))||(gpiod_line_request_output(led_test_line, "test", 0))<0){
         perror("Request line as event otput failed");
         goto release_line;
               }
    printf("Waiting for button press...\n");

    while (!handler_exit) {
        ret = gpiod_line_event_wait(button_line, NULL);
        if (ret < 0) {
            perror("Event wait failed");
            ret = -1;
	goto release_line;
        } 
        else if (ret == 0) {
            // infinite wait
              continue;
        }

        // Read event
        ret = gpiod_line_event_read(button_line, &event);
        if (ret < 0) {
            perror("Read event failed");
            ret = -1;
	goto release_line;
            
        }

        if (event.event_type == GPIOD_LINE_EVENT_RISING_EDGE) {
        printf("Button Pressed!\n");
        gpiod_line_set_value(led_press_line , 1);
        sleep(1); // sleep
        gpiod_line_set_value(led_press_line , 0);
            // Start TCP Server
            printf("Starting TCP server...\n");
            
            system("/usr/bin/tcp_server &");
            for (int i=0;i<50;i++)
            {
            gpiod_line_set_value(led_test_line, 1);
          gpiod_line_set_value(led_press_line, 0);
           sleep(1);

         gpiod_line_set_value(led_press_line, 1);
        gpiod_line_set_value(led_test_line, 0);
        sleep(1);
                         
            }        //pattern to indicate the server test is done
        }
    }
	
	
release_line:
         gpiod_line_release(button_line);
         gpiod_line_release(led_press_line);
	gpiod_line_release(led_test_line);
close_chip:
	gpiod_chip_close(chip);
end:
	return ret;
}

