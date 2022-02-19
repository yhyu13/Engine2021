:: unzip engine content
cd ./engine/
call unzip -q -o asset.zip 
call unzip -q -o vendors.zip 
cd ..

:: unzip sample project content
cd ./samples/demo/application
call unzip -q -o asset.zip 
cd ../../..

cd ./samples/asteriod/application
call unzip -q -o asset.zip 
cd ../../..
