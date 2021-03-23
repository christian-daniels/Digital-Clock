# Digital-Clock
An embedded system that displays the current time. Using an ATMega32 microcontroller, a keypad as input, and a LCD as output a user is able to set and view the date/time in real time.


## Functions
### Display time
The system displays a default time with form MM/DD/YYYY on the top row and HH:MM:SS on the bottom row.
### Update time
With each passing second the system updates the time on the screen, making updates where necessary.
### Set time
A user is able to set the time to any values they want by pressing \*. With this design decision the system verifies if an entered date/time is valid or not. If the date is not valid then the user is notified to enter a valid date. If the date is valid then the clock promptly begins ticking.
### Toggle between 12-hour and Military clocks
A user is able to switch clock modes by pressing D at any moment. There are two modes: 12-hour and 24-hour (military time)
## Main Modules Used
* ATMega32 Microcontroller
* 16 input Keypad
* 16 Character LCD
