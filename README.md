# Senior Project

Senior project by Connor Virostek and Reed Bowling.

## Dependencies

- GLFW 3
- PhysX 4.1
- Tested on MacOS

### Building PhysX on Mac
- Clone the latest PhysX wherever you want:
  - `git clone --depth 1 --branch 4.1 git@github.com:NVIDIAGameWorks/PhysX.git`
- Navigate to `PhysX/physx` and run `./generate_projects.sh`, choosing preset 1 (mac64) when prompted.
- Navigate to the directory where PhysXSDK.xcodeproj is generated and run `xcodebuild -project PhysXSDK.xcodeproj -alltargets -configuration release`
- Add `export PHYSX_DIR=path_to_your_physx_directory` to your shell config.
