# AimDotGame
The aiming dot game is based on the user hitting the shoot button when the blue dot in on the target consisting of three dots. 
Hitting the center gives 2 points while the sides give 1. 
One button will shoot and the other button will change the direction the aiming dot travels. 
Noise will cause the target to move faster while shaking will cause the aiming dot to move faster. 
The switch will turn on and off the game, toggling it on and off will reset the gamestate.

Inputs:
| Left button: | Shoot |
| Right button: | Change direction |
| Switch: | On/off also serves as game reset |
| Accelerometer: | Detects how unsteady the user is |
| Mic: | Detects noise |

Outputs:
| LEDs: | Represent dot positions as well as celebratory win/loss effects |
| Speaker: | Noise creates celebratory win/loss effects |
| Serial: | Broadcasts score as well as session high score |
