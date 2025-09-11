# retrotool
wip retroarch screenshot capture tool. I will announce 1.0<BR />
Modular. Main and main.hpp are excellent starters if you want to use SDL3 and Dear imgui.<BR />
![image](https://github.com/ZacharyGeurts/retrotool/blob/main/Screenshot%20from%202025-09-11%2012-59-40.png)<BR />
I am working on a Grok3 module.<BR />
Just add your hpp (and remove mine?)<BR />
My module sleeps for 3 or 5 seconds so it is less responsive than normal.<BR />
I figure it is slow enough to check a folder for new files.<BR />
See Lines 2, 16, and 49 in main.cpp to see how easy to start.<BR />
File monitor is written obtuse.<BR />
<BR />
Just ask an AI to write your window hpp and cpp.<BR />
`Write me a window for Dear imgui. I have main.cpp and main.h that handles all the setup with SDL3 and imgui.
file_monitor->renderUI(); is the call to open a new window in main.cpp.
You can reference this file_monitor.cpp.
paste file_monitor.cpp
I want blah blah blah`<BR />
<BR />
`git clone https://github.com/ZacharyGeurts/retrotool`<BR />
`cd retrotool`<BR />
`git clone https://github.com/ocornut/imgui`<BR />
`make`<BR />
`./retrotool`<BR />
<BR />
Requires SDL3.
