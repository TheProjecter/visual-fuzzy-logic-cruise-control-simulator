A student project hacked together within a couple of days.
It contains a [fuzzy logic](http://en.wikipedia.org/wiki/Fuzzy_logic) implementation with a cute little fuzzy rules parser. A very simple racing game was made to showcase fuzzy logic on [cruise control](http://en.wikipedia.org/wiki/Cruise_control).

Controls:
  * WSAD movement
  * C toggles cruise control and some fuzzy logic charts
  * O and P decreases and increases cruise control's speed, respectively
  * L reloads the rules and ABCD values defined in "rules.txt"
  * "Print Screen" key makes a screenshot
  * ESC quits the application

Important files:
  * rules.txt defines the rules and ABCD values
  * plugins.cfg, comment out either OpenGL or DX9, if you don't support it
  * ogre.cfg remove this file to be asked for rendering options at next run

Troubleshooting:
  * In case of "missing dx3d9 DLL" errors, comment out the first line in plugins.cfg -> `#Plugin=RenderSystem_Direct3D9_d`

If you have questions, please, contact us. We know our code isn't exactly the prettiest thing around ;)

![http://visual-fuzzy-logic-cruise-control-simulator.googlecode.com/files/ScreenShot.png](http://visual-fuzzy-logic-cruise-control-simulator.googlecode.com/files/ScreenShot.png)