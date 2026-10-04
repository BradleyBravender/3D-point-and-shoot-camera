# 3D-point-and-shoot-camera
For my University of Alberta Computer Engineering Capstone, I am individually developing a stereoscopic camera and accompanying rendering software from scratch. 

As an amateur mountaineer, I appreciate the ability to preview mountain objectives. However, the intricate details of handholds, slope, and other features of a route can be hardly discernable in a photograph. This project will address this problem by capturing the fine details of mountaineering routes in highly detailed, true-color 3D.

Far from existing 3D scanners and photogrammetry practices, my project is centered on the following value proposition:
* Inexpensive
* Easy to use, as in point-and-shoot with zero technical experience required
* GPU-friendly

## How 'from scratch'?
Nothing can truly be made from scratch, in the pure sense of the term. To strike a balance between expediency and the opportunity to learn, I decided to operate with the following constraints and level of abstraction:
* I must develop with the ESP-IDF framework, as the Arduino framework abstracts too much away
* I have free-reign access to the use of official ESP-IDF and ESP32 libraries
* I can consult online tutorials and public repositories, provided I properly cite any code I emulate
* I can only use AI in the cases of:
  * Tasks that do not provide educational value
  * Debugging when other means have been exhausted
  * Ideation when I have done prior research

## References
Instead of AI, I've consulted the following resources:
* [SD card example](https://randomnerdtutorials.com/esp32-microsd-card-arduino/)
* [Espressif camera driver](https://github.com/espressif/esp32-camera/tree/master)
* [ESP-IDF getting started guide](https://randomnerdtutorials.com/programming-esp32-esp-idf-vs-code/)
* [ESP-IDF Official Documentation](https://espressif-docs.readthedocs-hosted.com/projects/esp-idf/en/latest/)
* [FreeRTOS for ESP-IDF](https://controllerstech.com/esp32-freertos-multitasking-project/)
* 