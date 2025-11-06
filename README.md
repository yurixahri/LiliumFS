<img width="800" height="300" alt="logo" src="https://github.com/user-attachments/assets/14ca5df3-e19a-4b10-8485-0d4593584979" />

# Introduction
This a my personal project, based on HFS by [rejetto](https://github.com/rejetto/hfs). This project backend is written in C++ with QT framework, and Svelte for frontend.

Currently the project only support Windows, but i think this can be built in Linux with no problem.

# Quick Review
LiliumFS is a small, fast HTTP file server, easy to config and just one click to run.
* Access directly to your files from your disk.
* Freely add and custom your source name.
* Control permissions on each source.
* Can upload and delete (with permissions).

*TODO: Add zip function*
# How to use
Download the lasted release, extract to a folder and run the executable.
# Config
All config is in config.json file in the same app directory. the structure of this config is:
```
{
    "port": 9090,
    "encryption_key": "",
    "accounts": [],
    "files": [],
    "folders": [],
    "virtual":[]
}
```
### port
Default port is 9090, you can change the port in the config file or via admin panel. Note that i tested changing port while running the program, the program move to the new port but still dangling to old port, i still need to check this behaviour so it's safer to restart the program after changing port.
### encryption_key
A random generated key (its name should be salt, i'm just lazy to change), that use for hashing account password and sessionId. This key will be generated one when you first run the program.
### accounts
A list of created account, can be customized in admin panel.
### files and folders
Holding the list of source that you add from the admin panel, can be customized in admin panel. Each folder has 4 types of permissions: See, Download, Upload, Delete. Each file has 2 types of permissions: See and Download. Each permission can be customized as "Anyone", "No One", "Any account" or Selected accounts.
### virtual
A list of virtual root, each virtual root has 2 types of permissions: See and Download, and each can has it own files and folders list.

*Side note: Because i reserved "/ __ /" for api, if you name your source or virtual root "__", it will be skip and just call the api.*

# How to build
To build this project, please install msys2 and get the lasted [mingw-w64-x86_64-qt6-base](https://packages.msys2.org/packages/mingw-w64-x86_64-qt6-base) and [mingw-w64-x86_64-qt6-static](https://packages.msys2.org/packages/mingw-w64-x86_64-qt6-static), this is a static build project.
Get the lasted [qcoro](https://github.com/qcoro/qcoro), extract it to a folder, open msys2 mingw64 terminal in the folder and run:
```
mkdir build
cd build
cmake .. -G "Ninja" -DCMAKE_INSTALL_PREFIX=/mingw64 -DCMAKE_PREFIX_PATH=/mingw64
ninja -j3
ninja install -j1
```
If you don't have ninja please install ninja package from msys2.
Now you can go to your msys2 installed directory, go to **mingw64/bin** and find qtcreator.exe, open it and config Qt build kit and compiler. After that you can clone this project source code and open it in QtCreator and build it. Build binary will be located in `/build`.

For frontend, all the code is in `/client`. Frontend is a Sveltekit project, it's an easy framework, works normal with js, html and css. Get the lasted Nodejs and run `npm run install` to install all the modules, then you can run `npm run dev` to run the frontend. When testing most of the api will work on frontend except for login and logout function (because of mismatch hostname). To build the frontend run `npm run build`, this will create a single page in `/client/built`, copy all the contents inside build directory and move it to `/client`, next to the compiled qt program. The whole program structure is:
```
/client
   -> contains all the svelte kit build contents
liliumfs.exe
config.json
```

# API
You can look at the `main.cpp` source code for the api references, it speak for itself. There is some api like `/_app/*`, it's reserved for sveltekit SPA static files (because i make backend and frontend seperately).

# Final thought
This is a small project, mostly just for fun but its features are still helpful and easy to use. In the backend code i keep most config data in QJsonObject, so you may see huge nested loop just to modify a single json object. For the frontend, as i mentioned in the source code, if you want simplicity you can just modify endpoint for traditional MVC style (with inja), or htmx.
