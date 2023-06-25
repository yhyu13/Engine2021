:: unzip engine content
cd ./engine/
call tar -xf asset.zip 
call tar -xf vendors.zip 
cd ..

:: unzip sample project content
cd ./samples/demo/application
call tar -xf asset.zip 
cd ../../..

cd ./samples/asteriod/application
call tar -xf asset.zip 
cd ../../..
