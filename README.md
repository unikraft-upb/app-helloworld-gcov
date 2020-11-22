# Unikraft "hello-world" Application with Coverage Support (gcov)

This builds the Unikraft hello-world image with coverage support used by [gcov](https://gcc.gnu.org/onlinedocs/gcc/Gcov.html).
The actual support is provided in gcc via the `-fprofile-arcs` option.
Coverage support can be used for profile-guided optimization (PGO).
This is why the usual way to compile and link a program under test is via the `-fprofile-generate` option; it includes the `-fprofile-arcs` option.

Building and running the application can be done either using [kraft](https://github.com/unikraft/kraft) or with the manual approach.

## Dependencies

The following libraries / dependencies are required:
* `lib-pthread-embedded`
* `lib-libunwind`
* `lib-compiler-rt`
* `lib-libcxxabi`
* `lib-libcxx`
* `lib-newlib`
* `lib-gcc`

For `lib-gcc` use the [UPB fork](https://github.com/cs-pub-ro/lib-gcc), the `gcov` branch.
Similarly, for Unikraft use [UPB fork](https://github.com/cs-pub-ro/unikraft), the `gcov` branch.

For the other libraries, use the `staging` branch.

## Preparation

The profiling information is generated as a `.gcda` file.
The application requires filesystem support to generate / use the profiling information file.
The application will be configured to enable the use of the 9pfs filesystem.
9fps is a network filesystem that maps a local folder as the root directory in the application running in a KVM virtual machine.
We use the local folder `fs0` for this.
Create it using:
```
mkdir fs0
```

## Build using kraft

The easiest way to build and run the Unikraft image is by using [kraft](https://github.com/unikraft/kraft).

The custom UPB forks for Unikraft and `lib-gcc` are required. Create a custom folder structure for Unikraft and required dependencies:
* Clone the [UPB fork](https://github.com/cs-pub-ro/unikraft). The clone name has to be `unikraft` (the default name).
* Create a folder named `libs/`. The `unikraft` clone and the `libs/` folder are placed in the same folder.
* Clone the library repositories in the `libs/` folder.
  Be sure to name them according to `kraft`'s naming scheme: `pthread-embedded`, `libunwind`, `compiler-rt`, `libcxxabi`, `libcxx`, `newlib`, `gcc`.
  Use the `gcov` branch for `lib-gcc` library.
  Use the `staging` branch for the other libraries.

For `kraft` use the `UK_WORKDIR` kraft environment variable to point to the folder storing the `unikraft` clone and the `libs/` folder.
In the commands below, replace `/path/to/folder` with the actual path to this folder.

First configure the application:
```
UK_WORKDIR=/path/to/folder kraft configure -m x86_64 -p kvm
```

Then build the application:
```
UK_WORKDIR=/path/to/folder kraft build
```
The first time you do it will take some time, as the library files are downloaded, unpacked and built.
The resulting KVM image is `build/app-hello-world-gcov_kvm-x86_64`.

## Build using the manual approach

The manual approach gives more control on the configuration and build process.

Dependencies have to be included in the order in the `Makefile`.
Update the `Makefile` variables (`UK_ROOT`, `UK_LIBS`, `LIBS`) according to your setup.

Configure the application via the configuration screen:
```
make menuconfig
```

In the configuration menu, select `Library Configuration -> `vfscore: Configuration` -> `Default root device` and type `fs0`.
The rest of the configuration is loaded automatically from the `Config.uk` file.

Build the application by running
```
make
```
The first time you do it will take some time, as the library files are downloaded, unpacked and built.
The resulting KVM image is `build/app-hello-world-gcov_kvm-x86_64`.
The image name may be updated in the configuration screen, with the `Image name` option.

## Building for optimization

An application with coverage support can be built in two ways:
1. to generate the profiling information: the `-fprofile-generate` option
1. to use the generated information for optimization, i.e. the actual profile-guided optimization (PGO): the `-fprofile-use` option

By default, the application is built to generate the profile, i.e. using the `-fprofile-generate` option.

If building the application for optimization, i.e. using the `-fprofile-use` option, the configuration needs to be updated.

If using kraft, update the `kraft.yaml`.
Comment out the `CONFIG_OPTIMIZE_PGO_GENERATE=y` line and uncomment the `CONFIG_OPTIMIZE_PGO_USE=y` line.

If using the manual approach, enter the configuration screen (`make menuconfig`) and select `Build options` -> `PGO - Profile-guided optimization` -> `PGO Options` -> `profile-use`.

Then rebuild the image using kraft or the manual approach.

## Run

If using kraft, run the application with the command:
```
UK_WORKDIR=/path/to/folder kraft run -p kvm
```
Replace `/path/to/folder` with the actual path to the folder storing the `unikraft` clone and the `libs/` folder.

If using the manual approach, use the `qemu-guest` [script](https://github.com/unikraft/kraft/blob/staging/scripts/qemu-guest) in kraft:
```
qemu-guest -k build/app-hello-world-gcov_kvm-x86_64 -e fs0
```

Running the application, either using `kraft` or the `qemu-guest` script, requires `root` privileges.

After running the profile-generation image, the output file will be generated in `fs0/gcov_profiling/main.gcda`.
The file will be used by the profile-use phase after re-building the application for optimization.
The file is created with `root` privileges.
Modifying or removing it will require `root` privileges.
