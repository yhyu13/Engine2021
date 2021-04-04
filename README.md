#Long March Engine Project (Engine2021)

C++ 20 <br/>
OpenGL 4.5 <br/>
VS2019 16.8 <br/>
Win10 x64 <br/>

##Startup

The project contains source of the engine, and source sample applications.

1. Use `git lfs pull` to download ./engine/vendor.zip, ./engine/asset.zip and assets for sample projects.

2. Unzip vendor.zip, asset.zip, and optionally sample asssets. Beware to not create vendor/vendor/..., just vendor/... is fine.

3. Run `generate-project.bat` script to generate a MSCV solution for the application for each sample project.

4. To create your own application, please follow the same folder structure or change path settings in engine-config.json for each application. 

Optional: <br/>
Run index.html under engine/external/Remotery/vis/ for profiling. Beware that it may block other profiling tools such as Nvidia Nsight. 


###Sample screenshots

SSR
![demo_ssr](./samples/screenshots/demo_ssr.jpg)

SSDO
![demo_ssdo](./samples/screenshots/demo_ssdo.jpg)

![demo_ssdo_2](./samples/screenshots/demo_ssdo_2.jpg)

Motion Blur
![demo_motionblur](./samples/screenshots/demo_motionblur.png)

Editor
![demo_editor](./samples/screenshots/demo_editor.png)

Asteriod
![Asteriod_editor](./samples/screenshots/Asteriod_editor.png)

![Asteriod_gameplay](./samples/screenshots/Asteriod_gameplay.jpg)

##Special Thanks
Team G.S.W.Y @ DigiPen GAM541Fa19 and Team 4a games @ DigiPen GAM550Sp20 <br/>
Dushyant Shukla <br/>
Jie-Yang Tan <br/>
Kyle Wang <br/>
Taksh Goyal <br/>




