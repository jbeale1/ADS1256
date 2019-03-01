# ADS1256
This is a fork of Matt Bilsky's code for ADS1256 on Teensy/Arduino https://github.com/mbilsky/TeensyADS1256. I made only minor changes: the main loop calculates standard deviation and peak-to-peak amplitude of sets of 25 readings, and also tracks long-term drift relative to the first reading. 

Below is sample output from this code, measuring voltage on one NiCd AA battery at around 1.32 V. This is in differential mode with buffer on, PGA gain = 1. Each line combines 25 samples (10 sec at 2.5 Hz). The last three columns are in units of microvolts. I think it's pretty good performance from a $20 ADS1256 breakout board from Ebay.

<pre>
After self-cal: Offset Cal: -1434, Fullscale Cal: 3091162

   Time,      Volts,  stdev, pk-pk, drift
21:18:37.152, 1.3213073, 1.7, 6.5,  0.0
21:18:47.156, 1.3213095, 1.1, 4.2,  2.2
21:18:57.148, 1.3213062, 2.2, 8.3, -1.1
21:19:07.162, 1.3213060, 1.2, 3.6, -1.3
21:19:17.156, 1.3213074, 1.4, 5.9,  0.1
21:19:27.154, 1.3213054, 1.8, 6.5, -1.9
21:19:37.153, 1.3213063, 1.2, 4.2, -1.0
21:19:47.165, 1.3213057, 2.2, 7.7, -1.6
21:19:57.159, 1.3213041, 0.9, 3.6, -3.2
21:20:07.155, 1.3213056, 1.1, 4.2, -1.7
21:20:17.134, 1.3213046, 1.4, 5.9, -2.7
21:20:27.147, 1.3213044, 1.2, 4.8, -2.9
21:20:37.161, 1.3213050, 1.2, 5.4, -2.3
21:20:47.153, 1.3213060, 2.2, 7.1, -1.3
21:20:57.135, 1.3213051, 1.0, 3.6, -2.2
21:21:07.155, 1.3213030, 2.3, 8.9, -4.3
</pre>
