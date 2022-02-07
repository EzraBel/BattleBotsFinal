# BattleBots

https://youtu.be/9un575BmoaU

Battle Bots : Domo Arigato
Intro/Problem
This semester, we were tasked with designing and building a robot to compete in the internal Duke antweight battle bots competition. Simply put, our goal was to construct a robot weighing no more than 453 grams (1 lb) that would successfully win in a one-on-one battle against other robots. With our main goal being to win the competition, the team decided to focus on creating a human centered design. We believe that an easy and intuitive robot to control would ensure more wins. 

Constraints
For our build, we followed the constraints stated in the Standardized Procedure for the Advancement of Robotic Combat (SPARC) plastic class rule book. The most important rules are as follows:
 Robots must be ≤ 1lb (“antweight”) 
The “exposed battery” TKO rule will be enforced if the opposing robot has a weapon likely to puncture or damage the exposed battery
Matches will be 3 minutes in duration
The arena is a 4’ x 3’ x 2’ plywood and polycarbonate box with additional diagonal corner walls of approximately 12” in length
No “dead zones” or “push outs” are implemented
When a match does not end in KO of a robot, judging will proceed according to the SPARC Judging Guidelines, simplified criteria
SPARC rules state that the weapon for plastic class must be plastic, however for this course, the weapons can be constructed out of metal, but  this would add significantly more weight. Our robot also needed to consist of one superpower that would greatly advantage our robot.

Design Process
With a human centered design and the constraints  in mind, the team began working. We decided that we wanted a nimble/agile robot that would use a kinetic weapon and avoid brute force. After consulting with Professor Bletsch and viewing other robotics competitions, the team knew that oftentimes, robots would destroy themselves in battle as a result of weapons or inapt driving capabilities. This influenced us to build a more solid robot that was better skilled at defense instead of landing blows. 
After brainstorming sessions, we settled on a weapon that would use spinning to generate power. 
For the superpower, the team decided to create a new radio instead of using the one provided. Building our own radio would ensure that it was tailored to the robot and would also allow for easier control of the robot. 
The new radio uses a repurposed Logitech Extreme flight stick. The joystick was programmed using a raspberry pi zero and sends serial packages to the lora attached to it which then sends to the lora/arduino within the robot. Creating the code for our joystick allowed for the team to create fail sequences, power off certain parts of our robot, and scale the movements to what felt most comfortable. We expected it to give us an advantage over the other teams we would be fighting as there was no question what each button did and the logitech joystick was far less clunky than the drone controller. The use of Pygame for the program also allowed for the controls to seem similar to those of a video game, something that is far more recognizable than a drone controller. 
Electrical System
The wire harness for Domo Arigato consists of a battery, a switch, a brushless electronic speed controller (BLESC) and a brushed electronic speed controller (BESC). 
Domo Arigato uses a Lithium polymer (liPo) battery that connects to a switch and then to a brushless electronic speed controller (BLESC) and a brushed electronic speed controller (BESC). Because the receiver requires 5 V and our battery provides a bit more than that, we utilized the battery eliminator circuit to feed out the required 5V for the  BESC. The weapon motor is driven by a brushless motor that receives input from the BLESC. The N-20 motors used for driving are controlled by the BESC. 
The transmit LoRa which is connected to the Raspberry Pi via physical connection takes the values given to it by the Raspberry Pi and converts it to 8 bit. It is then packaged up and transmitted to the receive loRa via the FSK protocol. The receiver then takes the transmitted struct and converts each value into a PWM value within a designated range. This PWM value is sent out across one of 6 pins to which the receiver is connected to (12, 11, 10, 9, 6, 5) using the Servo library. 
Weaponry
As previously stated, the weapon for Domo Arigato is very simple. It is a rectangular piece of metal that sits atop the robot and spins similar to the blades of a helicopter. The weapon is driven via the brushless motor which is fitted to a 3d printed pulley. This pulley is connected to a second pulley via a belt which allows the weapon to spin. 

Performance
On December 04, 202, Domo Arigato participated in the first internal competition. Out of five battles, we won three. During the competition, we learned useful information about our robot that we did not anticipate before fighting. During the first fight, our radio lost connection to the LoRa on the robot. While planning out the design for the robot, we intended on programming kill sequences in the event that something like this happened. These sequences would either quit, kill the radio, resume radio, and reset props. The intended purpose of these commands was to pause/play the robot, to stop the weapon motor from spinning, and to shut down the robot whenever there was nothing being transmitted to the robot. During the competition, the kill sequence when nothing was being sent from the radio to the robot did not run as planned. This created very dangerous conditions as we could not shut down the robot and the weapon was engaged. 
Our second major issue was with the weapon. During testing, our weapon remained attached to the motor. However, during competition, after repeated contact with other robots, our pulley would break off and render our weapon useless. Because of the design of our robot, we could not deal damage once the weapon lost function. In addition, controls were not as human friendly as they could have been.

Changes
Based on the performance at the competition, I decided to tackle our  biggest problem, the radio. My main focus for this iteration was the controller as you can win a match with a malfunctioning weapon, but you cannot with a radio that is not transmitting. My first step was to try to recreate the bug. I achieved this through running the robot and then disrupting the connection between the raspberry pi and receiving loRa. Through this testing methodology, I noticed that when there was a loss of power while the raspberry pi was actively sending information there would be a communication issue the next time you switched on the robot via the switch.  	
After successfully recreating this bug, I hooked the raspberry pi, transmitting lora, and receiving loras up to monitors to witness what exactly was causing the bug on a simpler level. Through the use of simple print statements, I was able to determine when exactly the connection loss was occurring. 
For one test, I turned on power to the transmitting lora and raspberry pi and then switched on the receiving lora which is attached to the robot. I then switched on the weapon motor via the joystick and shortly after, cut the power to the pi/lora duo. As expected, the weapon motor continued to spin on the robot despite this not being the desired outcome. Upon looking at the output values for the receiving lora, I noticed that the receiving end continued to use the states that were being sent before the connection loss. 
After seeing where the error was occurring within the script, I began going through the code for the joystick, lora receive, and lora transmit file. Within the lora receive file, I noticed that there existed an if statement for when an error existed (for example, losing connection to the second lora and the raspberry pi), but the if statement resulted in an infinite loop being run. 

Solution
A simple solution to this error is to redo the if statement and implement a correct try catch. While debugging the code, I also noticed that in the event of a reboot or a loss of power, the default state for the receiving lora was not being set to off. I hypothesized that implementing failsafe logic similar to that of drone radio controllers along with fixing the infinite loop within the code would fix the communication issue. 
Through keeping track of the previous states of the entire system, I was able to effectively stop the motors when the connection is lost and guarantee that the default state on bootup is always motors off. 









