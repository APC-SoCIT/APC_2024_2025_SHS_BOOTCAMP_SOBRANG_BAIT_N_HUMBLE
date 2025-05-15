from datetime import datetime, time as dt_time
import time
from zoneinfo import ZoneInfo
import serial
#l;hasfhalsdalshfl;shf;lhsahfl;a;hlf;sldakhfdsafsaddfsd
arduino = serial.Serial(port='COM3', baudrate=9600, timeout=1)
time.sleep(2)
def time_set(shift, minute, second, hour):
    shift = shift.lower()
 
    if shift in ['am']:
        if hour == 12:
            hour = 0
    elif shift in ['pm']:
        if hour != 12:
            hour += 12
    else:
        print("Invalid shift. Use 'am' or 'pm'.")
        return None
 
    try:
        return dt_time(hour, minute, second)
    except ValueError:
        print("Invalid time values.")
        return None

hour = int(input("Hour (1–12): "))
minute = int(input("Minute: "))
second = int(input("Second: "))
shift = input("AM / PM: ")
 
convert_to_24_hours = time_set(shift, minute, second, hour)
 

if convert_to_24_hours is None:
    print("Invalid time settings.")
else:
    print(f"The light will turn off at {convert_to_24_hours} Philippine time.")
 
    philippine_time_zone = ZoneInfo("Asia/Manila")
 
    while True:
        now = datetime.now(philippine_time_zone).time()
        current_time_str = datetime.now(philippine_time_zone).strftime("%I:%M:%S %p")
        print(f"Current time: {current_time_str}", end="\r")
 
        if now >= convert_to_24_hours:
            print(f"\n⏰ Time reached! Light turned off at {current_time_str}.")
            arduino.write(b'1')
            break
 
        time.sleep(1)
 
 
 #lh;fadl;hlfahs;lfhsl;hfsa