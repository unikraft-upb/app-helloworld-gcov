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
UK_WORKDIR=/path/to/folder kraft configure
```

Then build the application:
```
UK_WORKDIR=/path/to/folder kraft build
```
The resulting KVM image is `build/app-hello-world-gcov_kvm-x86_64`.

## Build using the manual approach

The manual approach gives more control on the configuration and build process.

The dependencies have to be included in that order as done in the `Makefile`.
Update the `Makefile` variables (`UK_ROOT`, `UK_LIBS`, `LIBS`) according to your setup.

Configure the application via the configuration screen:
```
make menuconfig
```
Enable the following configuration options:
* `Platform Configuration` -> `KVM (guest)`
* `Library Configuration`
  * `libpthread-embedded`
  * `libunwind`
  * `compiler-rt`
  * `libcxxabi`
  * `libcxx`
  * `newlib`
  * `gcc`
    * `libgcov: A Profiling Library`
  * `vfscore: Configuration`
    * `Automatically mount a root filesystem (/)`
    * `Default root filesystem`: Choose `9pfs`
    * `Default root device`: Choose `fs0`
  * `POSIX sysinfo: Information about system parameters`
* `Build Options`
  * `PGO - Profile-guided optimization`

You can validate the configuration options, by diffing the `.config` file to the `config.sample` file:
```
diff .config config.sample
```

An application with coverage support can be built in two ways:
1. to generate the profiling information: the `-fprofile-generate` option
1. to use the generated information for optimization, i.e. the actual profile-guided optimization (PGO): the `-fprofile-use` option

By default, the application is built to generate the profile, i.e. using the `-fprofile-generate` option.

If building the application for optimization, i.e. using the `-fprofile-use` option, two configuration steps are required:
* In the configuration screen (`make menuconfig`), select `Build options` -> `PGO - Profile-guided optimization` -> `PGO Options` -> `profile-use`.
* In the `Makefile.uk` file, comment out the line with the `-fprofile-generate` option and uncomment the line with the `-fprofile-use` option.
  In the end the lines should be:
```
#APPHELLOWORLD_CFLAGS-$(CONFIG_OPTIMIZE_PGO_GENERATE) += -fprofile-generate
APPHELLOWORLD_CFLAGS-$(CONFIG_OPTIMIZE_PGO_USE) += -fprofile-use
```

Build the application by running
```
make
```
The first time you do it will take some time, as the library files are downloaded, unpacked and built.
The resulting KVM image is `build/app-hello-world-gcov_kvm-x86_64`.

## Run

The profiling information is generated as a `.gcda` file.
Create a folder named `fs0` where the file is to be stored:
```
mkdir fs0
```
The `fs0` will be mapped as the root directory in the Unikernel via the 9pfs filesystem support.

The Unikraft application is run in a KVM virtual machine.
Run it with `kraft` using the command:
```
UK_WORKDIR=/path/to/folder kraft run -p kvm
```
Replace `/path/to/folder` with the actual path to the folder storing the `unikraft` clone and the `libs/` folder.

Run it manually using the [`qemu-guest` script in kraft](https://github.com/unikraft/kraft/blob/staging/scripts/qemu-guest):
```
qemu-guest -k build/app-hello-world-gcov_kvm-x86_64 -e fs0
```
Running the application, either using `kraft` or the `qemu-guest` script, requires `root` privileges.

After running the profile generation image, the output file will be generated in `fs0/gcov_profiling/main.gcda`.
The file will be use by the profile use phase after re-building the application for optimization.
The file is created with `root` privileges.
Modifying or removing it will require `root` privileges.
