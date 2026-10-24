# Predictive Input Buffer — iOS port prototype

Target requested by user:
- Geometry Dash 2.208
- Geode 5.11.2
- iOS

## Important status

This archive is **source code, not a compiled `.geode` file**. It is an iOS-targeted prototype and is **not yet equivalent** to fraudian's Predictive Input Buffer. It demonstrates the iOS metadata/build layout and removes the Windows-only assumptions, but its current queue uses a fixed delay. The original's trajectory-based ghost-player simulation, survival scoring, and smart collision/object prediction still need to be ported and tested.

I am labeling this accurately rather than presenting a fixed-delay prototype as the requested finished mod.

## Build prerequisites

Building an iOS Geode mod requires a macOS development environment with Xcode/iOS toolchain and the matching Geode SDK. Set `GEODE_SDK` to the SDK directory, then configure/build using the Geode developer workflow for iOS.

## Notes for completing the real port

1. Port `TrajectoryBufferSim` from the original source without changing its prediction logic.
2. Replace `GetAsyncKeyState(VK_F6)` with an iOS-accessible Geode setting or in-game control.
3. Remove `Windows.h`, `AllocConsole`, `freopen_s`, `SetConsoleTitleA`, and Windows-only console output.
4. Validate the hooks and object layout against iOS GD 2.208 and Geode 5.11.2.
5. Test tap/release order, dual mode, wave mode, death/reset, and frame/tick synchronization on device.

Original repository: https://github.com/fraudian/Predictive-Input-Buffer

Please respect the original project's license and attribution terms when redistributing a derivative.


## Cloud build from an iPhone (GitHub Actions)

The repository includes `.github/workflows/build-ios.yml`. Once the project is in a GitHub repository:
1. Open the repository in Safari and upload the project files, preserving the `.github/workflows/build-ios.yml` path and the `src` folder.
2. Commit the files to the `main` branch.
3. Open the **Actions** tab and choose **Build Geode mod (iOS)**.
4. If prompted, enable workflows, then tap **Run workflow**.
5. Open the completed run, scroll to **Artifacts**, and download `PredictiveInputBuffer-iOS-build`.
6. Extract the artifact in the iPhone Files app.

If the build fails, open the run and copy the first actual compiler/configuration error. A successful compilation still does not establish feature equivalence or runtime stability.

Important: GitHub Actions can compile this prototype, but it cannot turn the current fixed-delay queue into the original trajectory-based predictor. That code still needs to be ported and validated.
