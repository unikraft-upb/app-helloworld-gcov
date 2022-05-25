# Unikraft Hello World Application with Coverage Support (gcov)

This builds the Unikraft helloworld image with coverage support used by [gcov](https://gcc.gnu.org/onlinedocs/gcc/Gcov.html).
The actual support is provided in gcc via the `-fprofile-arcs` option.
Coverage support can be used for profile-guided optimization (PGO) (via the `-fprofile-generate`) option, but it isn't considered here.
This is why the usual way to compile and link a program under test is via the `-fprofile-generate` option; it includes the `-fprofile-arcs` option.

Building and running the application can be done either using [kraft](https://github.com/unikraft/kraft) or with the manual approach.

## Dependencies

Apart from Unikraft, the following libraries / dependencies are required:
* `lib-pthread-embedded`
* `lib-libunwind`
* `lib-compiler-rt`
* `lib-libcxxabi`
* `lib-libcxx`
* `lib-newlib`
* `lib-gcc`

For `lib-gcc`, use the [UPB fork](https://github.com/cs-pub-ro/lib-gcc), the `gcov-arcs` branch.
For Unikraft and for the other libraries, use the `staging` or `stable` branch.

## Build using kraft

The easiest way to build and run the Unikraft image is by using [kraft](https://github.com/unikraft/kraft).

The custom UPB forks for `lib-gcc` is required.
Create a custom folder structure for Unikraft and required dependencies:
* Clone [Unikraft](https://github.com/unikraft/unikraft).
  The clone name has to be `unikraft` (the default name).
  The `stable` branch is known to work.
* Create a folder named `libs/`.
  The `unikraft` clone and the `libs/` folder are placed in the same folder.
* Clone the library repositories in the `libs/` folder.
  Be sure to name them according to `kraft`'s naming scheme: `pthread-embedded`, `libunwind`, `compiler-rt`, `libcxxabi`, `libcxx`, `newlib`, `gcc`.
  Use the `gcov-arcs` branch for `lib-gcc` library.
  Use the `staging` branch for the other libraries.

For `kraft`, use the `UK_WORKDIR` kraft environment variable to point to the folder storing the `unikraft` clone and the `libs/` folder.
In the commands below, replace `/path/to/folder` with the actual path to this folder.

First configure the application:
```
UK_WORKDIR=/path/to/folder kraft configure -m x86_64 -p kvm
```

Then build the application:
```
UK_WORKDIR=/path/to/folder kraft build
```
The first building of the application will take some time, as library files are downloaded, unpacked and built.
The resulting KVM image file is `build/app-helloworld-gcov_kvm-x86_64`.

## Build using the manual approach

The manual approach gives more control on the configuration and build process.

Dependencies have to be included in the order in the `Makefile`.
Update the `Makefile` variables (`UK_ROOT`, `UK_LIBS`, `LIBS`) according to your setup.

Configure the application via the configuration screen:
```
make menuconfig
```
Simply save the configuration and exit.
The configuration is loaded from the `Config.uk` file.

Build the application:
```
make
```
The first building of the application will take some time, as library files are downloaded, unpacked and built.
The resulting KVM image is `build/app-helloworld-gcov_kvm-x86_64`.
The image name may be updated in the configuration screen (`make menuconfig`), using the `Image name` option.

## Run

If using kraft, run the application with the command:
```
UK_WORKDIR=/path/to/folder kraft run -p kvm
```
Replace `/path/to/folder` with the path to the folder storing the `unikraft` clone and the `libs/` folder.

If using the manual approach, use the `qemu-guest` [script](https://github.com/unikraft/kraft/blob/staging/scripts/qemu-guest) in kraft:
```
qemu-guest -k build/app-helloworld-gcov_kvm-x86_64
```
Note that kraft also uses this script as part of its `kraft run` command.
So the `qemu-guest` command above can also be used for the kraft build.

Running the application, either using `kraft` or the `qemu-guest` script, requires `root` privileges.

The application (source code in `main.c`) prints out the total number of coverage counters for code snippets and functions calls in the program.
The program itself is built without coverage support, so no coverage counters are available for local functions.
To enable local coverage support, remove the `APPHELLOWORLD_CFLAGS-y` line in `Makefile.uk`.
