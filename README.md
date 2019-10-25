# solemix

Sole Mix is footwear that generates music from dance moves. It incorporates two force-sensitive resistors placed in the sole near the heel and toe areas of the foot, as well as an accelerometer to monitor the foot orientation. Data collected by all these sensors are mapped to different instrumental sounds and respond to kicking, tapping, and directional momentum. Through tracking different movements of the feet, the generated sounds flow together to create customized music. 

Two Adafruit Feather M0 microprocessors (with built-in WiFi modules) are used for the data transfer between the Sole Mix dance shoe and the sound playback device, such as a laptop. One Feather transfers the collected data from the sensors on the shoe to the other Feather, acting as a receiver unit wirelessly. The collected data is then passed to an audio processing software called Max Studio that maps the data to instrumental sounds for the collected data points. The sounds are then stitched together by the Max Studio to create a music file in real time, which can also be stored for later.

The product is for beginners, professional dancers, and choreographers alike. With Sole Mix, now everyone can dance to their own beat.
