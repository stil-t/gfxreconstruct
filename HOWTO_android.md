<!-- markdownlint-disable MD033 -->
<!-- markdownlint-disable MD041 -->
<p align="left"><img src="https://vulkan.lunarg.com/img/NewLunarGLogoBlack.png" alt="LunarG" width=263 height=113 /></p>

[![Creative Commons][1]][2]

[1]: https://i.creativecommons.org/l/by-nd/4.0/88x31.png "Creative Commons License"
[2]: https://creativecommons.org/licenses/by-nd/4.0/

Copyright &copy; 2023 LunarG, Inc.

# Android Walkthrough

## Index

1. [Purpose](#purpose)
2. [Building Vulkan Samples with the Capture Layer](#building-vulkan-samples-with-the-capture-layer)
3. [Capturing Vulkan Samples](#capturing-vulkan-samples)
4. [Capturing Released Application on Rooted System](#capturing-released-application-on-rooted-system)

## Purpose

The purpose of this doc is to show several examples in using the GFXReconstruct
tools.
These examples are intended to help work through issues you may discover while
attempting to perform the same process on your own application.

**NOTE:**
Many of these examples are given based on using a Linux system as the build
and/or host, but should be fairly easy to replicate behavior on a Windows
system with very little changes.

## Building Vulkan Samples with the Capture Layer

Let's look at adding the GFXReconstruct layer as a requirement of the 
[Khronos Vulkan Samples](https://github.com/KhronosGroup/Vulkan-Samples)
project.
These steps assume that you have all the dependencies necessary to build that
project, so refer to that repo to ensure those are met before continuing.

### 1. Pull down the GFXReconstruct source

If you haven't already, pull down the GFXReconstruct source from GitHub so that
it also updates any necessary submodules as part of the clone operation:

```bash
git clone --recurse-submodules https://github.com/LunarG/gfxreconstruct.git
```

The full location of the `gfxreconstruct` folder generated from this step
is to be used in all cases below for `{gfxreconstruct_root}`.

### 2. Pull down the source for Vulkan Samples

```bash
git clone --recurse-submodules https://github.com/KhronosGroup/Vulkan-Samples.git
```

### 3. Enter the Vulkan Samples source directory

```bash
cd Vulkan-Samples
```

### 4. Build the gradle files

```bash
./bldsys/scripts/generate_android_gradle.sh
```

### 5. Edit the top-level settings.gradle file
   (./build/android_gradle/settings.gradle)
   and add the following lines to the end

```text
include(':app',':VkLayer_gfxreconstruct')
project(':VkLayer_gfxreconstruct').projectDir = file('{gfxreconstruct_root}/android/layer')
```

**NOTE** Replacing {gfxreconstruct_root} with the location of the source you
cloned from the GFXReconstruct repo in step 1.

### 6. Add the capture layer to the samples application

Open ./build/android_gradle/app/build.gradle and add the following line at the
top of the `dependencies` block at the end of the file:

```text
    implementation project(':VkLayer_gfxreconstruct')
```

It should look something like this when you are done:

```text
dependencies {
    implementation project(':VkLayer_gfxreconstruct')
	implementation 'androidx.appcompat:appcompat:1.3.0'
	implementation 'com.google.android.material:material:1.4.0'
	implementation 'androidx.constraintlayout:constraintlayout:2.0.4'
}
```

### 7. Change to the appropriate build folder

From the top-level samples folder you now need to enter the following sub-
folder to build

```bash
cd build/android_gradle
```

### 8. Build the appropriate target

Some common targets are `assembleDebug` and `assembleRelease`.
We'll build for debug in this scenario:

```bash
gradle assembleDebug
```


## Capturing Vulkan Samples

This example follows using the Khronos Vulkan Samples built above and assumes
that you built the debug version.
The benefit of building the application in this way is that it will
automatically incorporate the GFXReconstruct capture layer into the .APK file
for the built application.

### 1. Run Logcat

```bash
adb logcat -s gfxrecon,vulkan
```

### 2. Install the debug version with storage access

Install the debug version of the samples we just built with the
`-g` option to request storage access:

```bash
adb install -g ./build/android_gradle/app/build/outputs/apk/debug/vulkan_samples-debug.apk
```

This should guarantee that the application (and any associated layers) is
allowed to write out anything generated.

### 3. Enable GFXReconstruct

Enable the GFXReconstruct layer just for the application using the global
settings Android provides:

```bash
adb shell settings put global enable_gpu_debug_layers 1
adb shell settings put global gpu_debug_app com.khronos.vulkan_samples
adb shell settings put global gpu_debug_layers "VK_LAYER_LUNARG_gfxreconstruct"
```

### 4. Set the capture options

Now, set the command to capture frames 500-700 so we can see what's happening
and write the file to the `/sdcard/Download` folder:

```bash
adb shell "setprop debug.gfxrecon.capture_file  '/sdcard/Download/samples_capture.gfxr'"
adb shell "setprop debug.gfxrecon.capture_frames '500-700'"
```

More capture options can be found in the [USAGE_android.md](./USAGE_android.md)
under the [Capture Options](./USAGE_android.md#capture-options) section.

### 5. Run the application

Keep the logcat output in a separate window/terminal because you can see
useful information there.

To run the application, I looked at the application list on the Android device,
then I clicked it to launch.
Android may ask you to "Allow access to manage all files".
When it asks you that, enable the option and click the back arrow to get to the
application.

Once launched, you will see something similar to the following in the `logcat`
window/terminal:

```text
D vulkan  : added global layer 'VK_LAYER_LUNARG_gfxreconstruct' from library '/data/app/~~3mMLAga5cfTsFt19iAcISQ==/com.khronos.vulkan_samples-cwdD6xdzmqmyo6QTZDsjng==/base.apk!/lib/arm64-v8a/libVkLayer_gfxreconstruct.so'
```

This message states that it found the GFXReconstruct capture layer in the
base.apk file which we built earlier, meaning there's no need to add an
additional external layer to any folders.

Now, click on one of the samples.
In this scenario, let's click on the "Dynamic uniform buffers" test.

Logcat should show:

```text
gfxrecon: Finished recording graphics API capture
```

Exit the application.


#### If the application rendered incorrectly during the capture

Running certain Vulkan samples tests results in corruption and require
additional changes to capture properly.
For example, if you attempted to capture the "Instancing" sample, it will render
without the asteroids.
This is because the GFXReconstruct layer needs some additional settings to
properly align and track the memory.

First, we will enable page guard on the memory:

```bash
adb shell "debug.gfxrecon.page_guard_persistent_memory true"
```

When this option is enabled, an allocation with a size equal to that of the
object being mapped is made once on the first map and is not freed until the
object is destroyed.
This option is intended to be used with applications that frequently map and
unmap large memory ranges, to avoid frequent allocation and copy operations that
can have a negative impact on performance.

Next, we will force page guard buffers are aligned to pages:

```bash
adb shell "debug.gfxrecon.page_guard_align_buffer_sizes true"
```

This option overrides the Vulkan API calls that report buffer memory properties
to report that buffer sizes and alignments must be a multiple of the system page
size.
This option is intended to be used with applications that perform CPU writes and
GPU writes/copies to different buffers that are bound to the same page of mapped
memory, which may result in data being lost when copying pages from the
`page_guard` shadow allocation to the real allocation.
This data loss can result in visible corruption during capture.
Forcing buffer sizes and alignments to a multiple of the system page size
prevents multiple buffers from being bound to the same page, avoiding data loss
from simultaneous CPU writes to the shadow allocation and GPU writes to the real
allocation for different buffers bound to the same page.

After making these changes, re-open the application and re-attempt recapture.


### 6. Verify the Capture File

I noticed in my capture `adb logcat` output that the file was recorded to
`/sdcard/Download`.
**NOTE**: It actually lists the full file in the message like the following:

```text
gfxrecon: Recording graphics API capture to /sdcard/Download/gfxrecon_capture_frames_500_through_700_20221211T130328.gfxr
```

But, let's say you didn't know, so double check that folder:

```bash
adb shell ls /sdcard/Download
```

The output might look like this:

```text
gfxrecon_capture_frames_500_through_700_20221211T130328.gfxr
```

There's our file right there.

### 7. Disable The GFXReconstruct layer

We want to make sure that the layer is not present for any further work we
are going to do.
In this case, we're going to check our replay, so disable any active debug
layers so they don't interfere with the tool.

```bash
adb shell settings put global enable_gpu_debug_layers 0
adb shell settings put global gpu_debug_layers ""
```

### 8. Install the replay application

Now, we need to install the Replay application that we built as part of the
GFXReconstruct source.
From the top of the source pulled down from the repo by using the
`gfxrecon.py` script:

```bash
./android/scripts/gfxrecon.py install-apk android/tools/replay/build/outputs/apk/debug/replay-debug.apk
```

### 9. Run the replay

Try running the replay using the `gfxrecon.py` script:

```bash
./android/scripts/gfxrecon.py replay /sdcard/Download/gfxrecon_capture_frames_500_through_700_20221211T130328.gfxr
```

Voila!
It worked, the replay executes properly.

### 10. Pull down the replay if needed

If you need to share your replay file, make sure to pull it off of the
device for sharing:

```bash
adb pull /sdcard/Download/gfxrecon_capture_frames_500_through_700_20221211T130328.gfxr
```

## Capturing Released Application On Rooted System

In this example, we'll capture a set of frames from the GPUScore benchmark.
This set of instructions assume that you have already have properly rooted
your device, reinstalled the proper Android image, and installed the target
application (in this case the benchmark).

### 1. Create an ADB shell with admin capabilities

```bash
adb shell
su
```

We'll refer to this as the admin ADB shell from here on out.

### 2. Find out full name of application

In another terminal/command window, run `logcat` to see application messages:

```bash
adb logcat -s vulkan
```

Then run the application and watch for message with the full name.

In my case, I see that the full application name is
`com.basemark.gpuscore.sacredpath`.

Alternatively, you can brows the list of installed applications:

```bash
adb shell cmd package list packages -3
```

### 3. Find out path for data

In the admin adb shell, run the following:

```bash
find /data/app -name *${Full Application Name}*
```

Where `${Full Application Name}` is the name we discovered in step 2 above.

On my system, running `find /data/app -name *com.basemark.gpuscore.sacredpath*`
returned:

```text
/data/app/~~CklMw7KUY3NvpJa1VxdzCg==/com.basemark.gpuscore.sacredpath-y4DeeR7lnUlvifTr1xSRWA==
```

### 4. Copy over the GFXReconstruct capture layer

In another terminal/command window, change directory to the location of your
GFXReconstruct source.
After building, perform the following command:

```bash
adb push \
    ./android/layer/build/intermediates/cmake/debug/obj/arm64-v8a/libVkLayer_gfxreconstruct.so \
    /storage/emulated/0/Download
```

Now, copy it into the application folder discovered in step 3 above using the
admin ADB shell:

```bash
mv /storage/emulated/0/Download/libVkLayer_gfxreconstruct.so \ 
       ${Application Data Path}/lib/arm64/libVkLayer_gfxreconstruct.so
```

Again, my full command was:

```bash
mv /storage/emulated/0/Download/libVkLayer_gfxreconstruct.so \ 
      /data/app/~~CklMw7KUY3NvpJa1VxdzCg==/com.basemark.gpuscore.sacredpath-y4DeeR7lnUlvifTr1xSRWA==/lib/arm64/libVkLayer_gfxreconstruct.so
```

Finally, make the capture layer executable:

```bash
chmod 777 /data/app/~~CklMw7KUY3NvpJa1VxdzCg==/com.basemark.gpuscore.sacredpath-y4DeeR7lnUlvifTr1xSRWA==/lib/arm64/libVkLayer_gfxreconstruct.so
```

### 5. Enable the layer for capture

I had to enable the layer system wide in this configuration and I also had to
disable the Vulkan usage for the UI or it would interfere with my capture:

```bash
adb shell "setprop debug.vulkan.layers 'VK_LAYER_LUNARG_gfxreconstruct'"
adb shell "setprop debug.hwui.renderer 'skiagl'"
```

### 6. Set GFXReconstruct capture options

Now, set the command to capture frames 100-200 so we can see what's happening:

```bash
adb shell "setprop debug.gfxrecon.capture_frames '100-200'"
```

And set the output filename to the proper location:

```bash
adb shell "setprop debug.gfxrecon.capture_file /storage/emulated/0/Download/sacredpath_capture.gfxr"
```

We will enable page guard on the memory:

```bash
adb shell "setprop debug.gfxrecon.page_guard_persistent_memory true"
```

When this option is enabled, an allocation with a size equal to that of the
object being mapped is made once on the first map and is not freed until the
object is destroyed.
This option is intended to be used with applications that frequently map and
unmap large memory ranges, to avoid frequent allocation and copy operations that
can have a negative impact on performance.

Next, we will force page guard buffers are aligned to pages:

```bash
adb shell "setprop debug.gfxrecon.page_guard_align_buffer_sizes true"
```

This option overrides the Vulkan API calls that report buffer memory properties
to report that buffer sizes and alignments must be a multiple of the system page
size.
This option is intended to be used with applications that perform CPU writes and
GPU writes/copies to different buffers that are bound to the same page of mapped
memory, which may result in data being lost when copying pages from the
`page_guard` shadow allocation to the real allocation.
This data loss can result in visible corruption during capture.
Forcing buffer sizes and alignments to a multiple of the system page size
prevents multiple buffers from being bound to the same page, avoiding data loss
from simultaneous CPU writes to the shadow allocation and GPU writes to the real
allocation for different buffers bound to the same page.

More capture options can be found in the [USAGE_android.md](./USAGE_android.md)
under the [Capture Options](./USAGE_android.md#capture-options) section.

### 7. Give the application permission to write

```bash
adb shell pm grant com.basemark.gpuscore.sacredpath android.permission.WRITE_EXTERNAL_STORAGE
```

### 8. Run the application

In my case, I opened up the menu, found the GPUScore application and chose
the official "Sacred Path" benchmark.

Watching the log output in the above `adb logcat` terminal, I waited until
the benchmark completed and then exited the application.

I made sure that I saw the following to make sure the recording started:

```text
I gfxrecon: Recording graphics API capture to /storage/emulated/0/Download/sacredpath_capture_frames_100_through_200_20221215T174939.gfxr
```

### 9. Verify the capture file

Check that the above file exists:

```bash
adb shell ls /storage/emulated/0/Download/sacredpath_capture_frames*.gfxr
```

### 10. Disable the capture layer

Disable the capture layer globally and also restore the Vulkan usage of the
HWUI:

```bash
adb shell "setprop debug.vulkan.layers ''"
adb shell "setprop debug.hwui.renderer 'skiavk'"
```

### 11. Install the replay application

Now, we need to install the Replay application that we built as part of the
GFXReconstruct source.
Install the replay APK from the root of the built source tree by using the
`gfxrecon.py` script:

```bash
./android/scripts/gfxrecon.py install-apk android/tools/replay/build/outputs/apk/debug/replay-debug.apk
```

### 12. Run the replay

Try running the replay using the `gfxrecon.py` script:

```bash
./android/scripts/gfxrecon.py replay /storage/emulated/0/Download/sacredpath_capture_frames_100_through_200_20221215T174939.gfxr
```